#include "cli_handler.h"
#include "aescrypt.h"
#include "gpgcontroller.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QDebug>
#include <openssl/opensslv.h>

int runCommandLineMode(const QStringList &args)
{
    if (args.size() < 3) {
        qCritical() << "Usage: <encrypt|decrypt> <input> <output> [password]";
        return 1;
    }

    QString mode = args[0];
    QString inputFile = args[1];
    QString outputFile = args[2];
    QString password = (args.size() > 3) ? args[3] : QString();

    AESCrypt crypt;
    bool success = false;

    if (mode == "encrypt") {
        success = crypt.encryptFile(inputFile, outputFile, password);
    } else if (mode == "decrypt") {
        if (inputFile.endsWith(".gpg", Qt::CaseInsensitive)) {
            GPGController gpg;

            QEventLoop loop;
            gpg.decryptFile(inputFile, outputFile,
                [&](const QString &log) { qInfo() << log; },
                [&](int progress) {
                    if (progress == 0) qInfo() << "Decrypt started...";
                    if (progress == 100) qInfo() << "Decrypt finished.";
                },
                [&](bool result) {
                    success = result;
                    loop.quit();
                });
            loop.exec();
        } else {
            success = crypt.decryptFile(inputFile, outputFile, password);
        }
    } else {
        qCritical() << "Mode must be 'encrypt' or 'decrypt'";
        return 1;
    }

    if (!success) {
        qCritical() << (mode == "encrypt" ? "Encryption" : "Decryption") << "failed.";
        return 1;
    }

    qInfo() << (mode == "encrypt" ? "Encryption" : "Decryption") << "succeeded.";
    return 0;
}

void showVersionInfo()
{
    QString versionText = QString("%1 version %2\nUsing OpenSSL: %3")
        .arg(QCoreApplication::applicationName())
        .arg(QCoreApplication::applicationVersion())
        .arg(OPENSSL_VERSION_TEXT);

    QString gpgPath =
#ifdef Q_OS_WIN
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe");
#else
        "gpg";
#endif

    QProcess gpgProc;
    gpgProc.setProgram(gpgPath);
    gpgProc.setArguments({"--version"});

    gpgProc.start();
    if (gpgProc.waitForStarted(3000) && gpgProc.waitForFinished(3000)) {
        QStringList lines = QString::fromUtf8(gpgProc.readAllStandardOutput()).split('\n');
        if (!lines.isEmpty() && lines[0].contains("gpg")) {
            QRegularExpression reGpg(R"(gpg\s+\(.*?\)\s+([0-9]+\.[0-9]+\.[0-9]+))");
            QRegularExpressionMatch match = reGpg.match(lines[0]);
            if (match.hasMatch())
                versionText += "\nGPG version: " + match.captured(1);
        }
        if (lines.size() > 1 && lines[1].contains("libgcrypt")) {
            QRegularExpression reLib(R"(libgcrypt\s+([0-9]+\.[0-9]+\.[0-9]+))");
            QRegularExpressionMatch match = reLib.match(lines[1]);
            if (match.hasMatch())
                versionText += "\nlibgcrypt version: " + match.captured(1);
        }
    } else {
        versionText += "\nGPG process could not be executed.";
    }

    // CLI 輸出
    qInfo().noquote() << versionText;
}
