#include "hashutil.h"
#include <QFile>
#include <QCryptographicHash>

HashAlgorithm toHashAlgorithm(const QString &algStr) {
    if (algStr == "MD5") return HashAlgorithm::MD5;
    if (algStr == "SHA-1") return HashAlgorithm::SHA1;
    if (algStr == "SHA-256") return HashAlgorithm::SHA256;
    if (algStr == "SHA-3-256") return HashAlgorithm::SHA3_256;
    return HashAlgorithm::MD5;  // 預設值
}

QString hashAlgorithmToString(HashAlgorithm algorithm) {
    switch (algorithm) {
        case HashAlgorithm::MD5: return "MD5";
        case HashAlgorithm::SHA1: return "SHA-1";
        case HashAlgorithm::SHA256: return "SHA-256";
        case HashAlgorithm::SHA3_256: return "SHA-3-256";
    }
    return "MD5";
}

QString HashUtil::computeHashFromText(const QString &text, HashAlgorithm algorithm) {
    QByteArray data = text.toUtf8();
    QCryptographicHash::Algorithm algo = QCryptographicHash::Md5;
    switch (algorithm) {
        case HashAlgorithm::MD5: algo = QCryptographicHash::Md5; break;
        case HashAlgorithm::SHA1: algo = QCryptographicHash::Sha1; break;
        case HashAlgorithm::SHA256: algo = QCryptographicHash::Sha256; break;
        case HashAlgorithm::SHA3_256: algo = QCryptographicHash::Sha3_256; break;
    }
    QCryptographicHash hash(algo);
    hash.addData(data);
    return hash.result().toHex();
}

QString HashUtil::computeHashFromFile(const QString &filePath, HashAlgorithm algorithm, std::function<void(int)> progressCallback)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash::Algorithm algo = QCryptographicHash::Md5;
    switch (algorithm) {
        case HashAlgorithm::MD5: algo = QCryptographicHash::Md5; break;
        case HashAlgorithm::SHA1: algo = QCryptographicHash::Sha1; break;
        case HashAlgorithm::SHA256: algo = QCryptographicHash::Sha256; break;
        case HashAlgorithm::SHA3_256: algo = QCryptographicHash::Sha3_256; break;
    }
    QCryptographicHash hash(algo);

    const qint64 totalBytes = file.size();
    qint64 processedBytes = 0;
    const qint64 bufferSize = 4096;

    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        if (buffer.isEmpty()) {
            break; // 讀取錯誤或EOF
        }

        hash.addData(buffer);
        processedBytes += buffer.size();

        if (progressCallback && totalBytes > 0) {
            int percent = static_cast<int>((processedBytes * 100) / totalBytes);
            progressCallback(std::min(percent, 100));
        }
    }

    if (progressCallback) {
        progressCallback(100); // 確保最後更新100%
    }

    return hash.result().toHex();
}