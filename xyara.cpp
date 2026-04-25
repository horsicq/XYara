/* Copyright (c) 2023-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xyara.h"

#include <cstdio>

namespace {
static const int YARA_SCAN_FLAGS = SCAN_FLAGS_REPORT_RULES_MATCHING | SCAN_FLAGS_REPORT_RULES_NOT_MATCHING;

FILE *openRulesFile(const QString &sFileName)
{
#ifdef Q_OS_WINDOWS
    return _wfopen(reinterpret_cast<const wchar_t *>(sFileName.utf16()), L"r");
#else
    return fopen(sFileName.toUtf8().constData(), "r");
#endif
}

QString yaraString(const char *pString)
{
    return QString::fromUtf8(pString ? pString : "");
}

class CompilerGuard {
public:
    ~CompilerGuard()
    {
        if (pCompiler != nullptr) {
            yr_compiler_destroy(pCompiler);
        }
    }

    YR_COMPILER *pCompiler = nullptr;
};

class RulesGuard {
public:
    ~RulesGuard()
    {
        if (pRules != nullptr) {
            yr_rules_destroy(pRules);
        }
    }

    YR_RULES *pRules = nullptr;
};

#if defined(_WIN32) || defined(__CYGWIN__)
class ScanFileHandle {
public:
    explicit ScanFileHandle(const QString &sFileName)
    {
        hFile = CreateFileW(reinterpret_cast<LPCWSTR>(sFileName.utf16()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            nullptr);
    }

    ~ScanFileHandle()
    {
        if (isValid()) {
            CloseHandle(hFile);
        }
    }

    bool isValid() const
    {
        return hFile != INVALID_HANDLE_VALUE;
    }

    HANDLE handle() const
    {
        return hFile;
    }

private:
    HANDLE hFile = INVALID_HANDLE_VALUE;
};
#else
class ScanFileHandle {
public:
    explicit ScanFileHandle(const QString &sFileName)
    {
        nFile = open(sFileName.toUtf8().constData(), O_RDONLY);
    }

    ~ScanFileHandle()
    {
        if (isValid()) {
            close(nFile);
        }
    }

    bool isValid() const
    {
        return nFile >= 0;
    }

    int handle() const
    {
        return nFile;
    }

private:
    int nFile = -1;
};
#endif
}  // namespace

XYara::XYara(QObject *pParent) : XThreadObject(pParent)
{
    m_emptyPdStruct = XBinary::createPdStruct();
    m_pPdStruct = &m_emptyPdStruct;
}

void XYara::initialize()
{
    yr_initialize();
}

void XYara::finalize()
{
    yr_finalize();
}

bool XYara::_handleRulesFile(YR_COMPILER *pYrCompiler, const QString &sFileName, const QString &sInfo)
{
    if (pYrCompiler == nullptr) {
        _reportError(QString(), tr("Invalid YARA compiler"));
        return false;
    }

    FILE *pFile = openRulesFile(sFileName);

    if (pFile == nullptr) {
        _reportError(sFileName, tr("Cannot open rules file"));
        return false;
    }

    const QByteArray baInfo = sInfo.toUtf8();
    const QByteArray baFileName = sFileName.toUtf8();
    const bool bResult = (yr_compiler_add_file(pYrCompiler, pFile, baInfo.constData(), baFileName.constData()) == ERROR_SUCCESS);

    fclose(pFile);

    return bResult;
}

XYara::SCAN_RESULT XYara::scanFile(const QString &sFileName, const QString &sFileNameOrDirectory, XBinary::PDSTRUCT *pPdStruct)
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    m_pPdStruct = (pPdStruct != nullptr) ? pPdStruct : &m_emptyPdStruct;
    m_nFreeIndex = XBinary::getFreeIndex(m_pPdStruct);
    XBinary::setPdStructInit(m_pPdStruct, m_nFreeIndex, 0);

    _resetScanResult(sFileName);
    m_mapFileNames.clear();

    const QString sRulesPath = XOptions::convertPathName(sFileNameOrDirectory);
    const QString sScanFileName = XOptions::convertPathName(sFileName);

    const auto finalizeScan = [&]() -> SCAN_RESULT {
        m_scanResult.nScanTime = scanTimer.elapsed();
        XBinary::setPdStructFinished(m_pPdStruct, m_nFreeIndex);
        m_nFreeIndex = -1;
        return m_scanResult;
    };

    CompilerGuard compiler;

    if (yr_compiler_create(&compiler.pCompiler) != ERROR_SUCCESS) {
        _reportError(QString(), tr("Cannot create YARA compiler"));
        return finalizeScan();
    }

    yr_compiler_set_callback(compiler.pCompiler, &XYara::_callbackCheckRules, this);

    qint32 nLoadedRuleFiles = 0;
    const QFileInfo rulesPathInfo(sRulesPath);

    if (rulesPathInfo.isDir()) {
        const QDir directory(sRulesPath);
        const QStringList listFiles = directory.entryList(QStringList() << "*.yar", QDir::Files, QDir::Name);

        for (const QString &sRelativeFileName : listFiles) {
            const QString sRuleFileName = directory.filePath(sRelativeFileName);
            const QString sBaseName = QFileInfo(sRuleFileName).baseName();

            if (_handleRulesFile(compiler.pCompiler, sRuleFileName, sBaseName)) {
                m_mapFileNames.insert(sBaseName, sRuleFileName);
                ++nLoadedRuleFiles;
            }
        }

        if (nLoadedRuleFiles == 0) {
            _reportError(sRulesPath, tr("No YARA rules were loaded"));
            return finalizeScan();
        }
    } else if (rulesPathInfo.isFile()) {
        const QString sBaseName = rulesPathInfo.baseName();

        if (!_handleRulesFile(compiler.pCompiler, sRulesPath, sBaseName)) {
            return finalizeScan();
        }

        m_mapFileNames.insert(sBaseName, sRulesPath);
    } else {
        _reportError(sRulesPath, tr("YARA rules path not found"));
        return finalizeScan();
    }

    RulesGuard rules;

    if (yr_compiler_get_rules(compiler.pCompiler, &rules.pRules) != ERROR_SUCCESS || (rules.pRules == nullptr)) {
        _reportError(sRulesPath, tr("Cannot build YARA rules"));
        return finalizeScan();
    }

    _setProgressTotal(rules.pRules->num_rules);

    ScanFileHandle fileHandle(sScanFileName);

    if (!fileHandle.isValid()) {
        _reportError(sScanFileName, tr("Cannot open scan target"));
        return finalizeScan();
    }

    const int nResult = yr_rules_scan_fd(rules.pRules, fileHandle.handle(), YARA_SCAN_FLAGS, &XYara::_callbackScan, this, 0);

    if ((nResult != ERROR_SUCCESS) && !((nResult == ERROR_CALLBACK_ERROR) && XBinary::isPdStructStopped(m_pPdStruct))) {
        _reportError(sScanFileName, QString("%1: %2").arg(tr("YARA scan failed"), QString::number(nResult)));
    }

    return finalizeScan();
}

void XYara::setData(const QString &sFileName, const QString &sRulesPath, XBinary::PDSTRUCT *pPdStruct)
{
    m_sFileName = sFileName;
    m_sRulesPath = sRulesPath;
    m_pPdStruct = (pPdStruct != nullptr) ? pPdStruct : &m_emptyPdStruct;
}

XYara::SCAN_RESULT XYara::getScanResult() const
{
    return m_scanResult;
}

XYara::SCAN_STRUCT XYara::getScanStructByUUID(const SCAN_RESULT *pScanResult, const QString &sUUID)
{
    XYara::SCAN_STRUCT result = {};

    if (pScanResult == nullptr) {
        return result;
    }

    qint32 nNumberOfRecords = pScanResult->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pScanResult->listRecords.at(i).sUUID == sUUID) {
            result = pScanResult->listRecords.at(i);
            break;
        }
    }

    return result;
}

QString XYara::getFileNameByRulesFileName(const QString &sRulesFileName) const
{
    return m_mapFileNames.value(sRulesFileName);
}

void XYara::process()
{
    scanFile(m_sFileName, m_sRulesPath, m_pPdStruct);
}

void XYara::_appendError(const QString &sRulesFile, const QString &sErrorString)
{
    if (sErrorString.isEmpty()) {
        return;
    }

    ERROR_RECORD errorRecord = {};
    errorRecord.sRulesFile = sRulesFile;
    errorRecord.sErrorString = sErrorString;

    m_scanResult.listErrors.append(errorRecord);
}

void XYara::_reportError(const QString &sRulesFile, const QString &sErrorString)
{
    _appendError(sRulesFile, sErrorString);

    if (sRulesFile.isEmpty()) {
        emit errorMessage(sErrorString);
    } else {
        emit errorMessage(QString("%1: %2").arg(sRulesFile, sErrorString));
    }
}

void XYara::_resetScanResult(const QString &sFileName)
{
    m_scanResult = {};
    m_scanResult.sFileName = sFileName;
}

void XYara::_setProgressTotal(qint32 nTotal)
{
    if ((m_pPdStruct == nullptr) || (m_nFreeIndex == -1)) {
        return;
    }

    XBinary::setPdStructTotal(m_pPdStruct, m_nFreeIndex, nTotal);
    XBinary::setPdStructStatus(m_pPdStruct, m_nFreeIndex, tr("Start"));
}

void XYara::_updateProgress(const QString &sStatusText)
{
    if ((m_pPdStruct == nullptr) || (m_nFreeIndex == -1)) {
        return;
    }

    XBinary::setPdStructCurrentIncrement(m_pPdStruct, m_nFreeIndex);
    XBinary::setPdStructStatus(m_pPdStruct, m_nFreeIndex, sStatusText);
}

void XYara::_callbackCheckRules(int error_level, const char *file_name, int line_number, const YR_RULE *rule, const char *message, void *user_data)
{
    Q_UNUSED(rule)

    XYara *pXYara = static_cast<XYara *>(user_data);

    if (pXYara == nullptr) {
        return;
    }

    const QString sFileName = yaraString(file_name);
    const QString sMessageText = yaraString(message);
    const QString sMessage = QString("%1: [%2] %3").arg(sFileName, QString::number(line_number), sMessageText);

    if (error_level == YARA_ERROR_LEVEL_ERROR) {
        pXYara->_reportError(sFileName, QString("[%1] %2").arg(QString::number(line_number), sMessageText));
    } else if (error_level == YARA_ERROR_LEVEL_WARNING) {
        emit pXYara->warningMessage(sMessage);
    }
    //    #define YARA_ERROR_LEVEL_ERROR   0
    //    #define YARA_ERROR_LEVEL_WARNING 1
    // qDebug("_callbackCheckRules");
}

int XYara::_callbackScan(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data)
{
    int nResult = CALLBACK_CONTINUE;

    XYara *pXYara = static_cast<XYara *>(user_data);

    if (pXYara == nullptr) {
        return nResult;
    }

    //    CALLBACK_MSG_RULE_MATCHING
    //    CALLBACK_MSG_RULE_NOT_MATCHING
    //    CALLBACK_MSG_SCAN_FINISHED
    //    CALLBACK_MSG_IMPORT_MODULE
    //    CALLBACK_MSG_MODULE_IMPORTED
    //    CALLBACK_MSG_TOO_MANY_MATCHES
    //    CALLBACK_MSG_CONSOLE_LOG
    if (message == CALLBACK_MSG_RULE_MATCHING) {
        YR_RULE *pYrRule = static_cast<YR_RULE *>(message_data);
#ifdef QT_DEBUG
        qDebug("%s", pYrRule->identifier);
#endif
        SCAN_STRUCT scanStruct = {};

        YR_STRING *pYrString = nullptr;
        YR_MATCH *pYrMatch = nullptr;
        const QString sRuleName = yaraString(pYrRule->identifier);
        const QString sRulesFile = (pYrRule->ns != nullptr) ? yaraString(pYrRule->ns->name) : QString();

        scanStruct.sUUID = XBinary::generateUUID();
        scanStruct.sRule = sRuleName;
        scanStruct.sRulesFile = sRulesFile;
        scanStruct.sRulesFullFileName = pXYara->getFileNameByRulesFileName(scanStruct.sRulesFile);

        if (pYrRule->strings != nullptr) {
            yr_rule_strings_foreach(pYrRule, pYrString)
            {
                yr_string_matches_foreach(context, pYrString, pYrMatch)
                {
                    SCAN_MATCH scanMatch = {};
                    scanMatch.sName = yaraString(pYrString->identifier);
                    scanMatch.nOffset = pYrMatch->offset;
                    scanMatch.nSize = pYrMatch->match_length;

                    scanStruct.listScanMatches.append(scanMatch);
                }
            }
        }

        pXYara->m_scanResult.listRecords.append(scanStruct);
        pXYara->_updateProgress(sRuleName);
    } else if (message == CALLBACK_MSG_RULE_NOT_MATCHING) {
        YR_RULE *pYrRule = static_cast<YR_RULE *>(message_data);
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("CALLBACK_MSG_RULE_NOT_MATCHING");
        pXYara->_updateProgress(yaraString(pYrRule->identifier));
    } else if (message == CALLBACK_MSG_TOO_MANY_MATCHES) {
        //        YR_STRING *pYrString = (YR_STRING *)message_data;
        // TODO warning
        //        qDebug("CALLBACK_MSG_TOO_MANY_MATCHES");
        emit pXYara->warningMessage("CALLBACK_MSG_TOO_MANY_MATCHES");
    } else if (message == CALLBACK_MSG_IMPORT_MODULE) {
        //        YR_MODULE_IMPORT *pModuleImport = (YR_MODULE_IMPORT *)message_data;
        //        qDebug("Module: %s", pModuleImport->module_name);
    } else if (message == CALLBACK_MSG_MODULE_IMPORTED) {
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("Module: %s", pYrStruncture->identifier);
    } else if (message == CALLBACK_MSG_CONSOLE_LOG) {
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("Module: %s", pYrStruncture->identifier);
        emit pXYara->infoMessage(yaraString(static_cast<char *>(message_data)));
    } else if (message == CALLBACK_MSG_SCAN_FINISHED) {
        // qDebug("CALLBACK_MSG_SCAN_FINISHED");
    }

    if (XBinary::isPdStructStopped(pXYara->m_pPdStruct)) {
        nResult = CALLBACK_ABORT;
    }

    // YR_OBJECT_STRUCTURE

    // TODO Check pdstruct
    // qDebug("_callbackScan: %d", message);

    return nResult;
}
