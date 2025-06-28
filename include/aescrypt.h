#ifndef AESCRYPT_H
#define AESCRYPT_H

#include <QObject>
#include <QString>

class AESCrypt : public QObject
{
    Q_OBJECT
public:
    explicit AESCrypt(QObject *parent = nullptr);

    bool encryptFile(const QString &inFile, const QString &outFile, const QString &password);
    bool decryptFile(const QString &inFile, const QString &outFile, const QString &password);

private:
    bool cryptFile(const QString &inFile, const QString &outFile, const QString &password, bool encrypt);

    static constexpr int KEY_LEN = 32;   // AES-256
    static constexpr int IV_LEN = 16;    // AES block size
    static constexpr int SALT_LEN = 8;
    static constexpr int PBKDF2_ITER = 100000;
};

#endif // AESCRYPT_H
