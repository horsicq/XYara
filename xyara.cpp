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
    g_pYrCompiler = nullptr;
    g_pRules = nullptr;
    yr_compiler_create(&g_pYrCompiler);
    yr_compiler_set_callback(g_pYrCompiler, &XYara::_callbackCheckRules, this);
}

XYara::~XYara()
{
    if (g_pRules) {
        yr_rules_destroy(g_pRules);
        g_pRules = nullptr;
    }

    yr_compiler_destroy(g_pYrCompiler);
}

void XYara::initialize()
{
    yr_initialize();
}

void XYara::finalize()
{
    yr_finalize();
}

bool XYara::addFile(QString sFileName)
{
    bool bResult = false;

    FILE *pFile;

    wchar_t *pFileNameW = new wchar_t[sFileName.length() + 1];
    sFileName.toWCharArray(pFileNameW);
    pFileNameW[sFileName.length()] = 0;

    pFile = _wfopen(pFileNameW , L"r");
    if (pFile != NULL)  {
        int nResult = yr_compiler_add_file(g_pYrCompiler, pFile, QFileInfo(sFileName).baseName().toLatin1().data(), sFileName.toLatin1().data());

        if (nResult == 0) {
            bResult = true;
        }

        fclose (pFile);
    }

    delete [] pFileNameW;

    if (g_pRules) {
        yr_rules_destroy(g_pRules);
        g_pRules = nullptr;
    }

    return bResult;
}

bool XYara::scanFile(QString sFileName)
{
    if (!g_pRules) {
        yr_compiler_get_rules(g_pYrCompiler, &g_pRules);
    }

    int nResult = yr_rules_scan_file(g_pRules, sFileName.toLatin1().data(), 0, &XYara::_callbackScan, this, 0);

    return (nResult == 0);
}

void XYara::_callbackCheckRules(int error_level, const char *file_name, int line_number, const YR_RULE *rule, const char *message, void *user_data)
{
//    #define YARA_ERROR_LEVEL_ERROR   0
//    #define YARA_ERROR_LEVEL_WARNING 1
    qDebug("_callbackCheckRules");
}

int XYara::_callbackScan(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data)
{
//    CALLBACK_MSG_RULE_MATCHING
//    CALLBACK_MSG_RULE_NOT_MATCHING
//    CALLBACK_MSG_SCAN_FINISHED
//    CALLBACK_MSG_IMPORT_MODULE
//    CALLBACK_MSG_MODULE_IMPORTED
//    CALLBACK_MSG_TOO_MANY_MATCHES
//    CALLBACK_MSG_CONSOLE_LOG
    if (message == CALLBACK_MSG_RULE_MATCHING) {
        YR_OBJECT_STRUCTURE *pYrStruncture = (YR_OBJECT_STRUCTURE *)message_data;
        qDebug("CALLBACK_MSG_RULE_MATCHING");

    } else if (message == CALLBACK_MSG_TOO_MANY_MATCHES) {
        YR_STRING *pYrString = (YR_STRING *)message_data;
        // TODO warning
        qDebug("CALLBACK_MSG_TOO_MANY_MATCHES");
    }

    // YR_OBJECT_STRUCTURE

    // TODO Check pdstruct
    qDebug("_callbackScan: %d", message);

    return CALLBACK_CONTINUE;
}

