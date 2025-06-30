#include "aescrypt.h"
#include <QFile>
#include <QCryptographicHash>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

AESCrypt::AESCrypt(QObject *parent) : QObject(parent) {}

// 提供加密和解密文件的接口
bool AESCrypt::encryptFile(const QString &inFile, const QString &outFile, const QString &password) {
    return encryptFile(inFile, outFile, password, nullptr); // 傳空 progress callback
}

bool AESCrypt::decryptFile(const QString &inFile, const QString &outFile, const QString &password) {
    return decryptFile(inFile, outFile, password, nullptr);
}

// 支持進度回調的加密和解密接口
bool AESCrypt::encryptFile(const QString &inFile, const QString &outFile, const QString &password,
                           std::function<void(int)> progressCallback)
{
    return cryptFile(inFile, outFile, password, true, progressCallback);
}

bool AESCrypt::decryptFile(const QString &inFile, const QString &outFile, const QString &password,
                           std::function<void(int)> progressCallback)
{
    return cryptFile(inFile, outFile, password, false, progressCallback);
}

// 實際的加密/解密邏輯
bool AESCrypt::cryptFile(const QString &inFile, const QString &outFile, const QString &password, bool encrypt, std::function<void(int)> progressCallback = nullptr)
{
    QFile in(inFile);
    QFile out(outFile);

    // 檢查輸入檔案是否存在
    if (!in.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open input file.");
        return false;
    }
    if (!out.open(QIODevice::WriteOnly)) {
        qWarning("Cannot open output file.");
        return false;
    }

    // 初始化 OpenSSL context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning("Failed to create EVP_CIPHER_CTX.");
        return false;
    }

    // 用8字節隨機鹽值
    unsigned char salt[SALT_LEN];
    if (encrypt) {
        if (RAND_bytes(salt, SALT_LEN) != 1) {
            qWarning("Failed to generate salt.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        out.write(reinterpret_cast<char *>(salt), SALT_LEN);
    } else {
        if (in.read(reinterpret_cast<char *>(salt), SALT_LEN) != SALT_LEN) {
            qWarning("Failed to read salt from input.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    }

    // 派生 key + iv
    unsigned char key[KEY_LEN], iv[IV_LEN];
    unsigned char keyiv[KEY_LEN + IV_LEN];
    if (PKCS5_PBKDF2_HMAC(password.toUtf8().constData(), password.toUtf8().length(),
                          salt, SALT_LEN,
                          PBKDF2_ITER,
                          EVP_sha256(),
                          KEY_LEN + IV_LEN,
                          keyiv) != 1) {
        qWarning("PBKDF2 key derivation failed.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    memcpy(key, keyiv, KEY_LEN);
    memcpy(iv, keyiv + KEY_LEN, IV_LEN);

    // 選擇 AES-256-CBC 密碼演算法
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();

    if (encrypt) {
        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
            qWarning("EVP_EncryptInit_ex failed.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    } else {
        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
            qWarning("EVP_DecryptInit_ex failed.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    }

    QByteArray inBuf, outBuf;
    outBuf.resize(4096 + EVP_MAX_BLOCK_LENGTH);

    int outLen = 0;

    // 計算總字節數以便進度回調
    qint64 totalBytes = in.size();
    qint64 processedBytes = 0;

    // 讀取輸入檔案並進行加密或解密
    while (!(in.atEnd())) {
        inBuf = in.read(4096);

        int ret = 0;
        if (encrypt) {
            ret = EVP_EncryptUpdate(ctx,
                reinterpret_cast<unsigned char*>(outBuf.data()), &outLen,
                reinterpret_cast<const unsigned char*>(inBuf.constData()), inBuf.size());
        } else {
            ret = EVP_DecryptUpdate(ctx,
                reinterpret_cast<unsigned char*>(outBuf.data()), &outLen,
                reinterpret_cast<const unsigned char*>(inBuf.constData()), inBuf.size());
        }

        if (ret != 1) {
            qWarning("EVP_EncryptUpdate / EVP_DecryptUpdate failed.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        out.write(outBuf.constData(), outLen);
        
        processedBytes += inBuf.size();
        if (progressCallback && totalBytes > 0) {
            int percent = static_cast<int>((processedBytes * 100) / totalBytes);
            progressCallback(qMin(percent, 100));
        }
    }

    int finalLen = 0;
    if (encrypt) {
        if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(outBuf.data()), &finalLen) != 1) {
            qWarning("EVP_EncryptFinal_ex failed.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        if (progressCallback) progressCallback(100);
    } else {
        if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(outBuf.data()), &finalLen) != 1) {
            qWarning("EVP_DecryptFinal_ex failed. 密碼錯誤或檔案損壞");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        if (progressCallback) progressCallback(100);
    }
    out.write(outBuf.constData(), finalLen);

    EVP_CIPHER_CTX_free(ctx);

    return true;
}
