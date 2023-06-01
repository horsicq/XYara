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
#ifndef XYARA_H
#define XYARA_H

#include "yara.h"
#include "xbinary.h"

class XYara : public QObject {
    Q_OBJECT
public:
    struct SCAN_MATCH {
        QString sName;
        qint64 nOffset;
        qint64 nSize;
    };

    struct SCAN_STRUCT {
        QString sUUID;
        QString sRule;
        QString sRulesFile;
        QList<SCAN_MATCH> listScanMatches;
    };

    struct ERROR_RECORD {
        QString sRulesFile;
        QString sErrorString;
    };

    struct DEBUG_RECORD {
        QString sRule;
        QString sRulesFile;
        qint64 nElapsedTime;
    };

    struct SCAN_RESULT {
        qint64 nScanTime;
        QString sFileName;
        QList<SCAN_STRUCT> listRecords;
        QList<ERROR_RECORD> listErrors;
        QList<DEBUG_RECORD> listDebugRecords;
    };

    explicit XYara(QObject *pParent = nullptr);
    ~XYara();

    static void initialize();
    static void finalize();

    SCAN_RESULT scanFile(const QString &sFileName);
    void setPdStruct(XBinary::PDSTRUCT *pPdStruct);
    void setData(const QString &sFileName);
    SCAN_RESULT getScanResult();
    bool setRulesFile(const QString &sFileName);
    void loadRulesFromFolder(const QString &sPathFileName);
    static SCAN_STRUCT getScanStructByUUID(SCAN_RESULT *pScanResult, const QString &sUUID);

public slots:
    void process();

private:
    bool _addRulesFile(const QString &sFileName);
    static void _callbackCheckRules(int error_level, const char *file_name, int line_number, const YR_RULE *rule, const char *message, void *user_data);
    static int _callbackScan(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data);

signals:
    void errorMessage(QString sErrorMessage);
    void warningMessage(QString sWarningMessage);
    void infoMessage(QString sInfoMessage);
    void completed(qint64 nElapsed);

private:
    YR_RULES *g_pRules;
    YR_COMPILER *g_pYrCompiler;
    XBinary::PDSTRUCT *g_pPdStruct;
    XBinary::PDSTRUCT g_pdStructEmpty;
    QString g_sFileName;
    SCAN_RESULT g_scanResult;
    QMap<QString, QString> g_mapFileNames;
};

#endif  // XYARA_H
