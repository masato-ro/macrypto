#include "keygen.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QtEndian>

bool generateRSAKeyPair(int bits, const QString &privPath, const QString &pubPath) {
    if (bits < 1024) { // 通常建議最小 1024 或 2048 位元
        qWarning() << "Error: RSA key bits too low. Minimum recommended is 1024 bits.";
        return false; // 直接返回失敗
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        qWarning() << "Failed to create EVP_PKEY_CTX";
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        qWarning() << "EVP_PKEY_keygen_init failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        qWarning() << "EVP_PKEY_CTX_set_rsa_keygen_bits failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        qWarning() << "EVP_PKEY_keygen failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    QFile privFile(privPath);
    if (!privFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open private key file for writing";
        EVP_PKEY_free(pkey);
        return false;
    }

    BIO *privBio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(privBio, pkey, nullptr, nullptr, 0, nullptr, nullptr);

    char *privPemData = nullptr;
    long privLen = BIO_get_mem_data(privBio, &privPemData);
    privFile.write((const char*)privPemData, privLen);
    privFile.close();
    BIO_free(privBio);

    QFile pubFile(pubPath);
    if (!pubFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open public key file for writing";
        EVP_PKEY_free(pkey);
        return false;
    }

    BIO *pubBio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(pubBio, pkey);

    char *pubPemData = nullptr;
    long pubLen = BIO_get_mem_data(pubBio, &pubPemData);
    pubFile.write((const char*)pubPemData, pubLen);
    pubFile.close();
    BIO_free(pubBio);

    EVP_PKEY_free(pkey);
    return true;
}

EVP_PKEY* loadPrivateKeyFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return nullptr;

    QByteArray keyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    if (!bio) return nullptr;

    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

QString generateOpenSSHPublicKey(EVP_PKEY *pkey, const QString &comment) {
    if (!pkey || EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA) {
        qWarning() << "Only RSA keys are supported for OpenSSH public key export.";
        return QString();
    }

    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    if (!rsa) {
        qWarning() << "Failed to get RSA from EVP_PKEY";
        return QString();
    }

    const BIGNUM *n = nullptr, *e = nullptr;
    RSA_get0_key(rsa, &n, &e, nullptr);

    if (!n || !e) {
        qWarning() << "Missing RSA key components.";
        RSA_free(rsa);
        return QString();
    }

    auto encode_mpint = [](const BIGNUM *bn) -> QByteArray {
        int len = BN_num_bytes(bn);
        QByteArray raw(len, 0);
        BN_bn2bin(bn, reinterpret_cast<unsigned char*>(raw.data()));

        // 若首位 bit 為 1，須補 0x00 以表示正整數
        if ((unsigned char)raw[0] & 0x80) {
            raw.prepend('\0');
        }

        quint32 net_len = qToBigEndian((quint32)raw.size());
        QByteArray result;
        result.append(reinterpret_cast<const char*>(&net_len), 4);
        result.append(raw);
        return result;
    };

    auto encode_string = [](const QByteArray &str) -> QByteArray {
        quint32 net_len = qToBigEndian((quint32)str.size());
        QByteArray result;
        result.append(reinterpret_cast<const char*>(&net_len), 4);
        result.append(str);
        return result;
    };

    QByteArray type = "ssh-rsa";
    QByteArray blob;
    blob += encode_string(type);
    blob += encode_mpint(e);
    blob += encode_mpint(n);

    QByteArray base64 = blob.toBase64();
    QString finalKey = "ssh-rsa " + QString::fromLatin1(base64);
        if (!comment.isEmpty()) {
        finalKey += " " + comment;
    }

    RSA_free(rsa);
    return finalKey;
}