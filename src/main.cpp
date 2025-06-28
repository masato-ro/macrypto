#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <openssl/opensslv.h>
#include "aescrypt.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 設定應用程式名稱和版本
    app.setApplicationName("aescrypt");
    app.setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("AES-256-CBC File Encrypt/Decrypt Tool");
    parser.addHelpOption();
    
    // 添加版本選項
    parser.addPositionalArgument("mode", "Mode of operation: encrypt or decrypt");
    parser.addPositionalArgument("input", "Path to the input file to encrypt/decrypt");
    parser.addPositionalArgument("output", "Path to the output file to write the result");
    parser.addPositionalArgument("password", "Password for AES encryption/decryption");
    
    QCommandLineOption versionOption(QStringList() << "v" << "version", "Show application and OpenSSL version");
    parser.addOption(versionOption);

    parser.process(app);

    // 檢查是否需要顯示版本
    if (parser.isSet(versionOption)) {
        qInfo().noquote() << QString("%1 version %2").arg(app.applicationName(), app.applicationVersion());
        qInfo().noquote() << QString("Using OpenSSL: %1").arg(OPENSSL_VERSION_TEXT);
        return 0;
    }

    // 若只有執行 ./aescrypt（無任何參數），則開啟 GUI 模式
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        MainWindow w;
        w.show();
        return app.exec();
    }

    // 檢查命令行參數
    if (args.size() != 4) {
        qCritical() << "Usage:\n  " << argv[0]
                    << " <encrypt|decrypt> <input> <output> <password>\n\n"
                    << "Description:\n"
                    << "  Encrypt or decrypt a file using AES-256-CBC with PBKDF2.\n\n"
                    << "Example:\n"
                    << "  " << argv[0] << " encrypt test.txt encrypted.bin mypassword\n"
                    << "  " << argv[0] << " decrypt encrypted.bin decrypted.txt mypassword";
        return 1;
    }

    QString mode = args[0];
    QString inputFile = args[1];
    QString outputFile = args[2];
    QString password = args[3];

    AESCrypt crypt;
    bool success = false;
    
    // 根據模式選擇加密或解密
    if (mode == "encrypt") {
        success = crypt.encryptFile(inputFile, outputFile, password);
    } else if (mode == "decrypt") {
        success = crypt.decryptFile(inputFile, outputFile, password);
    } else {
        qCritical() << "Mode must be 'encrypt' or 'decrypt'";
        return 1;
    }

    // 檢查操作是否成功    
    if (!success) {
        qCritical() << (mode == "encrypt" ? "Encryption" : "Decryption") << "failed";
        return 1;
    }

    qInfo() << (mode == "encrypt" ? "Encryption" : "Decryption") << "succeeded.";
    return 0;
}
