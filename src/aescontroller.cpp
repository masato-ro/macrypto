#include "aescontroller.h"
#include "aescrypt.h"

bool AESController::runEncrypt(const QString &input, const QString &output, const QString &password,
                               const std::function<void(int)> &progressCallback)
{
    AESCrypt crypt;
    return crypt.encryptFile(input, output, password, progressCallback);
}

bool AESController::runDecrypt(const QString &input, const QString &output, const QString &password,
                               const std::function<void(int)> &progressCallback)
{
    AESCrypt crypt;
    return crypt.decryptFile(input, output, password, progressCallback);
}
