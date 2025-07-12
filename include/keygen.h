#ifndef KEYGEN_H
#define KEYGEN_H

#include <QString>
#include <openssl/evp.h>

bool generateRSAKeyPair(int bits, const QString &privPath, const QString &pubPath);

EVP_PKEY* loadPrivateKeyFromFile(const QString &filePath);

QString generateOpenSSHPublicKey(EVP_PKEY *pkey, const QString &comment);

#endif // KEYGEN_H
