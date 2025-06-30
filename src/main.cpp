#include <QApplication>
#include <QCommandLineParser>
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <openssl/opensslv.h>
#include "aescrypt.h"
#include "mainwindow.h"

bool decryptGPG(const QString &inFile, const QString &outFile)
{
    if (!QFile::exists(inFile)) {
        qWarning() << "Input file does not exist:" << inFile;
        return false;
    }

    QString gpgPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe");

    QStringList args;
    args << "--yes"
         << "-o" << outFile
         << "-d" << inFile;

    QProcess gpg;
    gpg.start(gpgPath, args);

    if (!gpg.waitForFinished(-1)) {
        qWarning() << "gpg process timeout or failed";
        return false;
    }

    QByteArray stderrOutput = gpg.readAllStandardError();
    if (!stderrOutput.isEmpty()) {
        qWarning() << "gpg error output:" << stderrOutput;
    }

    if (gpg.exitCode() != 0) {
        qWarning() << "gpg exited with code" << gpg.exitCode();
        return false;
    }

    QFile outFileObj(outFile);
    if (!outFileObj.exists() || outFileObj.size() == 0) {
        qWarning() << "Output file missing or empty:" << outFile;
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("windows");

    app.setApplicationName("aescrypt");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Macrypt - AES/GPG Encrypt/Decrypt Tool");
    parser.addHelpOption();

    parser.addPositionalArgument("mode", "Mode of operation: encrypt or decrypt");
    parser.addPositionalArgument("input", "Path to the input file to encrypt/decrypt");
    parser.addPositionalArgument("output", "Path to the output file");
    parser.addPositionalArgument("password", "Password for encryption/decryption");

    QCommandLineOption versionOption(QStringList() << "v" << "version", "Show application and OpenSSL version");
    parser.addOption(versionOption);

    parser.process(app);

   if (parser.isSet(versionOption)) {
        QString versionText = QString("%1 version %2\nUsing OpenSSL: %3")
                            .arg(app.applicationName())
                            .arg(app.applicationVersion())
                            .arg(OPENSSL_VERSION_TEXT);

        QProcess gpgProc;
        QString gpgPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe");
        gpgProc.setProgram(gpgPath);
        gpgProc.setArguments(QStringList() << "--version");

        gpgProc.start();
        if (gpgProc.waitForStarted(3000)) {
            if (gpgProc.waitForFinished(3000)) {
                QByteArray gpgOutput = gpgProc.readAllStandardOutput();
                QStringList lines = QString::fromUtf8(gpgOutput).split('\n');

                // 解析 GPG 主版本
                if (!lines.isEmpty() && lines[0].contains("gpg")) {
                    QRegularExpression reGpg(R"(gpg\s+\(.*?\)\s+([0-9]+\.[0-9]+\.[0-9]+))");
                    QRegularExpressionMatch matchGpg = reGpg.match(lines[0]);
                    if (matchGpg.hasMatch()) {
                        versionText += "\nGPG version: " + matchGpg.captured(1);
                    }
                }

                // 解析 libgcrypt 版本
                if (lines.size() > 1 && lines[1].contains("libgcrypt")) {
                    QRegularExpression reLib(R"(libgcrypt\s+([0-9]+\.[0-9]+\.[0-9]+))");
                    QRegularExpressionMatch matchLib = reLib.match(lines[1]);
                    if (matchLib.hasMatch()) {
                        versionText += "\nlibgcrypt version: " + matchLib.captured(1);
                    }
                }
            } else {
                versionText += "\nGPG process did not finish in time.";
            }
        } else {
            versionText += "\nFailed to start gpg process.";
        }

        QMessageBox::information(nullptr, "Version", versionText);
        return 0;
    }

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        MainWindow w;
        w.show();
        return app.exec();
    }

    if (args.size() < 3) {
        qCritical() << "Usage:\n  " << argv[0]
                    << " <encrypt|decrypt> <input> <output> [password]";
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
            // 不直接傳密碼，改為 GPG 自行呼叫 pinentry
            success = decryptGPG(inputFile, outputFile);
        } else {
            success = crypt.decryptFile(inputFile, outputFile, password);
        }
    } else {
        qCritical() << "Mode must be 'encrypt' or 'decrypt'";
        return 1;
    }

    qInfo() << (mode == "encrypt" ? "Encryption" : "Decryption") << "succeeded.";
    return 0;
}