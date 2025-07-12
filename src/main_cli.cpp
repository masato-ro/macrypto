#include <QCoreApplication>
#include "cli_handler.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("aescrypt");
    app.setApplicationVersion("1.0");

    QStringList args = app.arguments();
    args.pop_front(); // 移除執行檔名稱

    if (args.contains("-v") || args.contains("--version")) {
        showVersionInfo();
        return 0;
    }

    return runCommandLineMode(args);
}
