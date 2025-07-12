#include "GPGController.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug> // 確保包含 QDebug，雖然本段代碼沒有直接使用 QDebug::qDebug()

GPGController::GPGController(QObject *parent) : QObject(parent) {}

void GPGController::decryptFile(const QString &inputFile, const QString &outputFile,
                                std::function<void(const QString &)> logCallback,
                                std::function<void(int)> progressCallback,
                                std::function<void(bool)> finishedCallback)
{
    QString gpgPath = QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe";
    QFileInfo gpgExe(gpgPath);
    if (!gpgExe.exists()) {
        if (logCallback) logCallback("❗找不到 gpg.exe！");
        if (finishedCallback) finishedCallback(false);
        return;
    }

    if (gpgProcess) {
        gpgProcess->deleteLater();
        gpgProcess = nullptr;
    }

    gpgProcess = new QProcess(this);

    // 用於累積 GPG 的標準錯誤輸出
    QString *accumulatedErrorOutput = new QString(); 

    // 連接標準輸出：GPG 的正常輸出通常在這裡，例如解密進度或確認訊息
    connect(gpgProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = gpgProcess->readAllStandardOutput();
        if (logCallback) logCallback(QString::fromUtf8(output));
    });

    // 連接標準錯誤：GPG 的警告、提示和錯誤訊息會從這裡輸出
    connect(gpgProcess, &QProcess::readyReadStandardError, this, [=]() {
        QByteArray err = gpgProcess->readAllStandardError();
        QString currentErrorChunk = QString::fromUtf8(err);
        
        // 累積所有 GPG 的標準錯誤輸出，不在此處即時顯示
        *accumulatedErrorOutput += currentErrorChunk;
    });

    // 進程結束時的處理邏輯
    connect(gpgProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus status) {
            if (progressCallback) {
                progressCallback(100); // 進度完成
            }
            bool success = (exitCode == 0 && status == QProcess::NormalExit);

            // 總是先顯示所有累積的 GPG 原始標準錯誤輸出 (如果有的話)
            if (!accumulatedErrorOutput->isEmpty() && logCallback) {
                logCallback("\n--- GPG 原始輸出 ---\n" + *accumulatedErrorOutput);
            }

            // 如果解密成功，並且原始輸出中包含 MDC 警告，則顯示自定義警告
            if (success && accumulatedErrorOutput->contains("message was not integrity protected", Qt::CaseInsensitive)) {
                if (logCallback) {
                    logCallback("⚠️ **警告：您的 GPG 檔案沒有訊息完整性保護 (MDC)！**\n這表示檔案在解密前可能已被修改。雖然已成功解密，但請注意此風險。");
                }
            }

            // 根據解密結果顯示最終訊息
            if (success) {
                if (logCallback) logCallback("✅ 解密成功！");
                // 成功解密後清除 GPG 密碼快取
                QString agentPath = QCoreApplication::applicationDirPath() + "/gpg/bin/gpg-connect-agent.exe";
                QProcess::startDetached(agentPath, QStringList() << "reloadagent" << "/bye");
                if (logCallback) logCallback("🔒 已清除 GPG 密碼快取（下次會重新要求輸入）。");
            } else {
                if (logCallback) logCallback("❌ 解密失敗！");
                // 解密失敗也嘗試清除 GPG 密碼快取
                QString agentPath = QCoreApplication::applicationDirPath() + "/gpg/bin/gpg-connect-agent.exe";
                QProcess::startDetached(agentPath, QStringList() << "reloadagent" << "/bye");
                if (logCallback) logCallback("🔒 已清除 GPG 密碼快取（下次會重新要求輸入）。");
            }
            if (finishedCallback) finishedCallback(success); // 告知調用者解密是否成功

            // 清理資源
            gpgProcess->deleteLater();
            gpgProcess = nullptr;
            delete accumulatedErrorOutput; // 釋放累積緩衝區的記憶體
        });

    // 開始進度，通常用於UI顯示不確定進度
    if (progressCallback) {
        progressCallback(0); // <-- 這裡觸發 0%
    }
    
    // **重要：請確保這裡有 --ignore-mdc-error 參數！**
    QStringList args = {"--yes", "-o", outputFile, "-d", "--ignore-mdc-error", inputFile}; // 加上 --ignore-mdc-error
    gpgProcess->start(gpgPath, args);
}