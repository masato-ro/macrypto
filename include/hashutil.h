#ifndef HASHUTIL_H
#define HASHUTIL_H

#include <QString>

enum class HashAlgorithm {
    MD5,
    SHA1,
    SHA256,
    SHA3_256
};

class HashUtil {
public:
    static QString computeHashFromText(const QString &text, HashAlgorithm algorithm);

    static QString computeHashFromFile(const QString &filePath, HashAlgorithm algorithm,
                                       std::function<void(int)> progressCallback);
};

#endif // HASHUTIL_H
