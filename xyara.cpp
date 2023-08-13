/* Copyright (c) 2023 hors<horsicq@gmail.com>
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

XYara::XYara(QObject *pParent) : QObject(pParent)
{
    g_pdStructEmpty = XBinary::createPdStruct();
    g_pPdStruct = &g_pdStructEmpty;
    g_pYrCompiler = nullptr;
    g_pRules = nullptr;
    g_scanResult = {};
}

XYara::~XYara()
{
    //    if (g_pRules) {
    //        yr_rules_destroy(g_pRules);
    //        g_pRules = nullptr;
    //    }

    //    if (g_pYrCompiler) {
    //        yr_compiler_destroy(g_pYrCompiler);
    //        g_pYrCompiler = nullptr;
    //    }
}

void XYara::initialize()
{
    yr_initialize();
}

void XYara::finalize()
{
    yr_finalize();
}

bool XYara::_addRulesFile(const QString &sFileName)
{
    bool bResult = false;

    FILE *pFile = nullptr;

#ifdef Q_OS_WINDOWS
    wchar_t *pFileNameW = new wchar_t[sFileName.length() + 1];
    sFileName.toWCharArray(pFileNameW);
    pFileNameW[sFileName.length()] = 0;
    pFile = _wfopen(pFileNameW, L"r");
#else
    pFile = fopen(sFileName.toUtf8().data(), "r");
#ifdef QT_DEBUG
    qDebug("bool XYara::_addRulesFile(const QString &sFileName)");
#endif
#endif
    if (pFile != NULL) {
        QString sBaseName = QFileInfo(sFileName).baseName();
        int nResult = yr_compiler_add_file(g_pYrCompiler, pFile, sBaseName.toUtf8().data(), sFileName.toUtf8().data());

        if (nResult == 0) {
            g_mapFileNames.insert(sBaseName, sFileName);
            bResult = true;
        }

        fclose(pFile);
    }

#ifdef Q_OS_WINDOWS
    delete[] pFileNameW;
#endif

    //    if (g_pRules) {
    //        yr_rules_destroy(g_pRules);
    //        g_pRules = nullptr;
    //    }

    return bResult;
}

XYara::SCAN_RESULT XYara::scanFile(const QString &sFileName)
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    if (!g_pRules) {
        yr_compiler_get_rules(g_pYrCompiler, &g_pRules);
    }

    g_scanResult = {};
    // TODO flags
    //    _CrtMemState s1,s2,s3;
    //    _CrtMemCheckpoint( &s1 );
    int nResult = yr_rules_scan_file(g_pRules, sFileName.toUtf8().data(), 0, &XYara::_callbackScan, this, 0);
    //    _CrtMemCheckpoint( &s2 );
    //    if ( _CrtMemDifference( &s3, &s1, &s2) )
    //       _CrtMemDumpStatistics( &s3 );

    g_scanResult.sFileName = sFileName;
    g_scanResult.nScanTime = scanTimer.elapsed();

    return g_scanResult;
}

void XYara::setPdStruct(XBinary::PDSTRUCT *pPdStruct)
{
    g_pPdStruct = pPdStruct;
}

void XYara::setData(const QString &sFileName)
{
    g_sFileName = sFileName;
}

XYara::SCAN_RESULT XYara::getScanResult()
{
    return g_scanResult;
}

bool XYara::setRulesFile(const QString &sFileName)
{
    if (g_pRules) {
        yr_rules_destroy(g_pRules);
        g_pRules = nullptr;
    }

    if (g_pYrCompiler) {
        yr_compiler_destroy(g_pYrCompiler);
        g_pYrCompiler = g_pYrCompiler;
    }

    g_mapFileNames.clear();

    yr_compiler_create(&g_pYrCompiler);
    yr_compiler_set_callback(g_pYrCompiler, &XYara::_callbackCheckRules, this);

    return _addRulesFile(sFileName);
}

void XYara::loadRulesFromFolder(const QString &sPathFileName)
{
    if (g_pRules) {
        yr_rules_destroy(g_pRules);
        g_pRules = nullptr;
    }

    if (g_pYrCompiler) {
        yr_compiler_destroy(g_pYrCompiler);
        g_pYrCompiler = nullptr;
    }

    g_mapFileNames.clear();

    yr_compiler_create(&g_pYrCompiler);
    yr_compiler_set_callback(g_pYrCompiler, &XYara::_callbackCheckRules, this);

    QDir directory(sPathFileName);

    QList<QString> listFiles = directory.entryList(QStringList() << "*.yar", QDir::Files);

    qint32 nNumberOfFiles = listFiles.count();

    for (qint32 i = 0; i < nNumberOfFiles; i++) {
        _addRulesFile(sPathFileName + QDir::separator() + listFiles.at(i));
    }
}

XYara::SCAN_STRUCT XYara::getScanStructByUUID(SCAN_RESULT *pScanResult, const QString &sUUID)
{
    XYara::SCAN_STRUCT result = {};

    qint32 nNumberOfRecords = pScanResult->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pScanResult->listRecords.at(i).sUUID == sUUID) {
            result = pScanResult->listRecords.at(i);
            break;
        }
    }

    return result;
}

QString XYara::getFileNameByRulesFileName(const QString &sRulesFileName)
{
    return g_mapFileNames.value(sRulesFileName);
}

void XYara::process()
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    int nFreeIndex = XBinary::getFreeIndex(g_pPdStruct);
    XBinary::setPdStructInit(g_pPdStruct, nFreeIndex, 0);

    scanFile(g_sFileName);

    XBinary::setPdStructFinished(g_pPdStruct, nFreeIndex);

    emit completed(scanTimer.elapsed());
}

void XYara::_callbackCheckRules(int error_level, const char *file_name, int line_number, const YR_RULE *rule, const char *message, void *user_data)
{
    Q_UNUSED(rule)

    XYara *_pXYara = (XYara *)user_data;
    QString sString = QString("%1: [%2] %3").arg(QString(file_name), QString::number(line_number), QString(message));

    if (error_level == YARA_ERROR_LEVEL_ERROR) {
        emit _pXYara->errorMessage(sString);
    } else if (error_level == YARA_ERROR_LEVEL_WARNING) {
        emit _pXYara->warningMessage(sString);
    }
    //    #define YARA_ERROR_LEVEL_ERROR   0
    //    #define YARA_ERROR_LEVEL_WARNING 1
    // qDebug("_callbackCheckRules");
}

int XYara::_callbackScan(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data)
{
    int nResult = CALLBACK_CONTINUE;

    XYara *_pXYara = (XYara *)user_data;
    //    CALLBACK_MSG_RULE_MATCHING
    //    CALLBACK_MSG_RULE_NOT_MATCHING
    //    CALLBACK_MSG_SCAN_FINISHED
    //    CALLBACK_MSG_IMPORT_MODULE
    //    CALLBACK_MSG_MODULE_IMPORTED
    //    CALLBACK_MSG_TOO_MANY_MATCHES
    //    CALLBACK_MSG_CONSOLE_LOG
    if (message == CALLBACK_MSG_RULE_MATCHING) {
        YR_RULE *pYrRule = (YR_RULE *)message_data;

        SCAN_STRUCT scanStruct = {};

        YR_STRING *pYrString = nullptr;
        YR_MATCH *pYrMatch = nullptr;

        scanStruct.sUUID = XBinary::generateUUID();
        scanStruct.sRule = pYrRule->identifier;
        scanStruct.sRulesFile = pYrRule->ns->name;
        scanStruct.sRulesFullFileName = _pXYara->getFileNameByRulesFileName(scanStruct.sRulesFile);

        if (pYrRule->strings != nullptr) {
            yr_rule_strings_foreach(pYrRule, pYrString)
            {
                yr_string_matches_foreach(context, pYrString, pYrMatch)
                {
                    SCAN_MATCH scanMatch = {};
                    scanMatch.sName = pYrString->identifier;
                    scanMatch.nOffset = pYrMatch->offset;
                    scanMatch.nSize = pYrMatch->match_length;

                    scanStruct.listScanMatches.append(scanMatch);
                }
            }
        }

        _pXYara->g_scanResult.listRecords.append(scanStruct);
    } else if (message == CALLBACK_MSG_RULE_NOT_MATCHING) {
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("CALLBACK_MSG_RULE_NOT_MATCHING");
    } else if (message == CALLBACK_MSG_TOO_MANY_MATCHES) {
        //        YR_STRING *pYrString = (YR_STRING *)message_data;
        // TODO warning
        //        qDebug("CALLBACK_MSG_TOO_MANY_MATCHES");
        emit _pXYara->warningMessage("CALLBACK_MSG_TOO_MANY_MATCHES");
    } else if (message == CALLBACK_MSG_IMPORT_MODULE) {
        //        YR_MODULE_IMPORT *pModuleImport = (YR_MODULE_IMPORT *)message_data;
        //        qDebug("Module: %s", pModuleImport->module_name);
    } else if (message == CALLBACK_MSG_MODULE_IMPORTED) {
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("Module: %s", pYrStruncture->identifier);
    } else if (message == CALLBACK_MSG_CONSOLE_LOG) {
        //        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        //        qDebug("Module: %s", pYrStruncture->identifier);
        emit _pXYara->infoMessage((char *)message_data);
    } else if (message == CALLBACK_MSG_SCAN_FINISHED) {
        // qDebug("CALLBACK_MSG_SCAN_FINISHED");
    }

    if (_pXYara->g_pPdStruct->bIsStop) {
        nResult = CALLBACK_ABORT;
    }

    // YR_OBJECT_STRUCTURE

    // TODO Check pdstruct
    // qDebug("_callbackScan: %d", message);

    return nResult;
}
