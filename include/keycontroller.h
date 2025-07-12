#pragma once

#include <QString>
#include <openssl/evp.h>

class KeyController {
public:
    bool generate(int bits, const QString &privPath, const QString &pubPath, bool convertToSSH, const QString &comment = "");

    QString getLastError() const;
    QString getSSHPublicKey() const;
    QString getSSHOutputPath() const;

private:
    QString lastError;
    QString sshPublicKey;
    QString sshOutputPath;
};
