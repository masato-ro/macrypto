#ifndef HASHCONTROLLER_H
#define HASHCONTROLLER_H

#include <QString>
#include <functional>
#include "hashutil.h"

class HashController {
public:
    static QString runHashFromText(const QString &text, HashAlgorithm algorithm);
    static QString runHashFromFile(const QString &filePath, HashAlgorithm algorithm,
                                   std::function<void(int)> progressCallback = nullptr);
};

#endif // HASHCONTROLLER_H
