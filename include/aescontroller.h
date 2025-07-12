#ifndef AESCONTROLLER_H
#define AESCONTROLLER_H

#include <QString>
#include <functional>

class AESController
{
public:
    static bool runEncrypt(const QString &input, const QString &output, const QString &password,
                           const std::function<void(int)> &progressCallback);

    static bool runDecrypt(const QString &input, const QString &output, const QString &password,
                           const std::function<void(int)> &progressCallback);
};

#endif // AESCONTROLLER_H