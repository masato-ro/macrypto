#include <QApplication>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "cli_handler.h"  // 只用來呼叫 showVersionInfo()

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("windows");

    app.setApplicationName("aescrypt");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Macrypt - AES/GPG Encrypt/Decrypt Tool");
    parser.addHelpOption();

    QCommandLineOption versionOption(QStringList() << "v" << "version", "Show application and OpenSSL version");
    parser.addOption(versionOption);

    parser.process(app);

    if (parser.isSet(versionOption)) {
        showVersionInfo();
        return 0;
    }

    MainWindow w;
    w.show();
    return app.exec();
}
