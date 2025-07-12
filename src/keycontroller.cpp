#include "keycontroller.h"
#include "keygen.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSysInfo>

bool KeyController::generate(int bits, const QString &privPath, const QString &pubPath, bool convertToSSH, const QString &comment) {
    lastError.clear();
    sshPublicKey.clear();
    sshOutputPath.clear();

    if (!generateRSAKeyPair(bits, privPath, pubPath)) {
        lastError = "無法產生金鑰對。";
        return false;
    }

    if (convertToSSH) {
        EVP_PKEY *pkey = loadPrivateKeyFromFile(privPath);
        if (!pkey) {
            lastError = "載入私鑰失敗，無法轉換為 OpenSSH 格式。";
            return false;
        }

        QString finalComment = comment.trimmed();
        if (finalComment.isEmpty())
            finalComment = QSysInfo::machineHostName();

        sshPublicKey = generateOpenSSHPublicKey(pkey, finalComment);
        EVP_PKEY_free(pkey);

        if (sshPublicKey.isEmpty()) {
            lastError = "轉換為 OpenSSH 公鑰失敗。";
            return false;
        }

        QString baseName = QFileInfo(pubPath).completeBaseName();
        sshOutputPath = QFileInfo(pubPath).absolutePath() + "/" + baseName + ".pub";

        QFile file(sshOutputPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            lastError = "無法寫入 OpenSSH 公鑰檔案：" + sshOutputPath;
            return false;
        }

        QTextStream out(&file);
        out << sshPublicKey;
        file.close();
    }

    return true;
}

QString KeyController::getLastError() const {
    return lastError;
}

QString KeyController::getSSHPublicKey() const {
    return sshPublicKey;
}

QString KeyController::getSSHOutputPath() const {
    return sshOutputPath;
}
