#include "hashcontroller.h"

QString HashController::runHashFromText(const QString &text, HashAlgorithm algorithm)
{
    if (text.trimmed().isEmpty()) {
        return QString();
    }
    return HashUtil::computeHashFromText(text, algorithm);
}

QString HashController::runHashFromFile(const QString &filePath, HashAlgorithm algorithm,
                                        std::function<void(int)> progressCallback)
{
    return HashUtil::computeHashFromFile(filePath, algorithm, progressCallback);
}
