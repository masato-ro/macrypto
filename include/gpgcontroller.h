#ifndef GPGCONTROLLER_H
#define GPGCONTROLLER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <functional>

class GPGController : public QObject
{
    Q_OBJECT
public:
    explicit GPGController(QObject *parent = nullptr);

    // 啟動解密流程，傳入輸入輸出路徑，和輸出日誌的回調函式，回調字串為日誌內容
    void decryptFile(const QString &inputFile, const QString &outputFile,
                     std::function<void(const QString &)> logCallback,
                     std::function<void(int)> progressCallback,
                     std::function<void(bool success)> finishedCallback);

private:
    QProcess *gpgProcess = nullptr;
};

#endif // GPGCONTROLLER_H
