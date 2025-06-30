#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aescrypt.h"
#include "hashutil.h"
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QFileInfo>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Macrypt - AES/GPG Encrypt/Decrypt Tool"));
    
    ui->comboAlgorithm->addItems(QStringList() << "MD5" << "SHA-1" << "SHA-256" << "SHA-3-256");
    ui->lineEditFilePath->installEventFilter(this);
    ui->lineEditFilePath->setAcceptDrops(true);
}

MainWindow::~MainWindow() {
    delete ui;
}

// 將字串轉換為 HashAlgorithm 枚舉
static HashAlgorithm toHashAlgorithm(const QString &algStr) {
    if (algStr == "MD5") return HashAlgorithm::MD5;
    if (algStr == "SHA-1") return HashAlgorithm::SHA1;
    if (algStr == "SHA-256") return HashAlgorithm::SHA256;
    if (algStr == "SHA-3-256") return HashAlgorithm::SHA3_256;
    return HashAlgorithm::MD5; // 預設
}

void MainWindow::on_pushButton_start_clicked() {
    QString input = ui->lineEdit_input->text();
    QString output = ui->lineEdit_output->text();
    QString password = ui->lineEdit_password->text();
    QString mode = ui->comboBox_mode->currentText();

    ui->progressBar->setValue(0);
    ui->textEdit_log->append(QString("開始 %1 作業...").arg(mode));
    ui->textEdit_log->append(QString("輸入檔案: %1").arg(input));
    ui->textEdit_log->append(QString("輸出檔案: %1").arg(output));

    if (!QFile::exists(input)) {
        ui->textEdit_log->append("❌ 錯誤：輸入檔案不存在！");
        return;
    }

    AESCrypt crypt;
    bool ok = false;

    if (mode == "Encrypt") {
        ui->textEdit_log->append("🔐 進行加密中...");
        ok = crypt.encryptFile(input, output, password, [=](int percent) {
            ui->progressBar->setValue(percent);
            QCoreApplication::processEvents();
        });
    } else {
        ui->textEdit_log->append("🔓 進行解密中...");
        ok = crypt.decryptFile(input, output, password, [=](int percent) {
            ui->progressBar->setValue(percent);
            QCoreApplication::processEvents();
        });
    }

    if (ok) {
        ui->textEdit_log->append(QString("✅ %1 成功！").arg(mode));
    } else {
        ui->textEdit_log->append(QString("❌ %1 失敗，請確認密碼與檔案內容。").arg(mode));
    }

    ui->progressBar->setValue(100);
}

void MainWindow::on_pushButton_browseInput_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, "Select input file");
    if (!filename.isEmpty()) {
        ui->lineEdit_input->setText(filename);
    }
}

void MainWindow::on_pushButton_browseOutput_clicked() {
    QString filename = QFileDialog::getSaveFileName(this, "Select output file");
    if (!filename.isEmpty()) {
        ui->lineEdit_output->setText(filename);
    }

}

void MainWindow::on_btnHashText_clicked()
{
    QString inputText = ui->plainTextInput->toPlainText();
    QString algorithmStr = ui->comboAlgorithm->currentText();
    HashAlgorithm algorithm = toHashAlgorithm(algorithmStr);

    QString result = HashUtil::computeHashFromText(inputText, algorithm);
    ui->textEditHashResult->setPlainText(result);
}

void MainWindow::on_btnSelectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("選擇檔案"));
    if (!fileName.isEmpty()) {
        ui->lineEditFilePath->setText(fileName);
    }
}

void MainWindow::on_btnHashFile_clicked()
{
    QString fileName = ui->lineEditFilePath->text();
    QString algorithmStr = ui->comboAlgorithm->currentText();
    HashAlgorithm algorithm = toHashAlgorithm(algorithmStr);

    ui->progressBar_2->setValue(0);

    QString result = HashUtil::computeHashFromFile(fileName, algorithm, [&](int percent){
        ui->progressBar_2->setValue(percent);
        QCoreApplication::processEvents();  // 確保 UI 即時更新
    });

    ui->textEditHashResult->setPlainText(result);
}

void MainWindow::on_pushButton_copyHash_clicked() {
    QString hashText = ui->textEditHashResult->toPlainText();
    if (hashText.isEmpty()) {
        QMessageBox::warning(this, tr("錯誤"), tr("沒有可複製的 Hash 結果。"));
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(hashText);
    QMessageBox::information(this, tr("成功"), tr("Hash 結果已複製到剪貼簿。"));
}

void MainWindow::on_pushButtonGPG_browseInput_clicked() {
    QString file = QFileDialog::getOpenFileName(this, tr("選擇加密檔案 (*.gpg)"));
    if (!file.isEmpty())
        ui->lineEditGPG_input->setText(file);
}

void MainWindow::on_pushButtonGPG_browseOutput_clicked() {
    QString file = QFileDialog::getSaveFileName(this, tr("選擇輸出檔案"));
    if (!file.isEmpty())
        ui->lineEditGPG_output->setText(file);
}

void MainWindow::on_pushButton_decryptGPG_clicked() {
    QString input = ui->lineEditGPG_input->text();
    QString output = ui->lineEditGPG_output->text();
    QString password = ui->lineEditGPG_password->text();

    ui->textEditGPG_log->clear();
    ui->progressBarGPG->setValue(0);

    if (input.isEmpty() || output.isEmpty() || password.isEmpty()) {
        ui->textEditGPG_log->append("❗請填入所有欄位！");
        return;
    }

    QString gpgPath = QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe";
    QFileInfo gpgExe(gpgPath);
    if (!gpgExe.exists()) {
        ui->textEditGPG_log->append("❗找不到 gpg.exe！");
        return;
    }

    // 建立 QProcess 執行 gpg
    QProcess *gpgProc = new QProcess(this);
    QStringList args = {
        "--batch",
        "--yes",
        "--passphrase", password,
        "-o", output,
        "-d", input
    };

    ui->textEditGPG_log->append("🔐 開始解密中...");
    ui->progressBarGPG->setRange(0, 0); // 不確定進度

    connect(gpgProc, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = gpgProc->readAllStandardOutput();
        ui->textEditGPG_log->append(QString::fromUtf8(output));
    });

    connect(gpgProc, &QProcess::readyReadStandardError, this, [=]() {
        QByteArray err = gpgProc->readAllStandardError();
        ui->textEditGPG_log->append(QString::fromUtf8(err));
    });

    connect(gpgProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus status) {
            ui->progressBarGPG->setRange(0, 1);
            ui->progressBarGPG->setValue(1);

            if (exitCode == 0 && status == QProcess::NormalExit) {
                ui->textEditGPG_log->append("✅ 解密成功！");
            } else {
                ui->textEditGPG_log->append("❌ 解密失敗！");
            }

            gpgProc->deleteLater();
        });

    gpgProc->start(gpgPath, args);
}

void MainWindow::on_actionExit_triggered() {
    qApp->quit();
}

void MainWindow::on_actionAbout_triggered() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("關於"));
    dialog->resize(500, 350);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QString gpgVersionText;
    QProcess gpgProc;
    QString gpgPath = QCoreApplication::applicationDirPath() + "/gpg/bin/gpg.exe";
    gpgProc.start(gpgPath, QStringList() << "--version");

    if (gpgProc.waitForFinished(1000)) {
        QStringList lines = QString::fromUtf8(gpgProc.readAllStandardOutput()).split('\n');
        if (!lines.isEmpty()) {
            QRegularExpression reGpg(R"(gpg\s+\(.*?\)\s+([0-9]+\.[0-9]+\.[0-9]+))");
            QRegularExpressionMatch matchGpg = reGpg.match(lines.value(0));
            if (matchGpg.hasMatch())
                gpgVersionText += "GPG: " + matchGpg.captured(1);

            QRegularExpression reLib(R"(libgcrypt\s+([0-9]+\.[0-9]+\.[0-9]+))");
            QRegularExpressionMatch matchLib = reLib.match(lines.value(1));
            if (matchLib.hasMatch())
                gpgVersionText += "<br>libgcrypt: " + matchLib.captured(1);
        }
    } else {
        gpgVersionText = tr("GPG 資訊無法讀取");
    }

    QString aboutText = tr(
        "<b>Macrypt - AES/GPG Encrypt/Decrypt Tool</b> (Version %4)<br><br>"
        "This project is developed using Qt and OpenSSL, and GnuPG.<br><br>"
        "<b>Used libraries and licenses:</b><br>"
        "Qt: %1<br>"
        "License: LGPL<br><br>"
        "OpenSSL: %2<br>"
        "License: Apache License 2.0<br>"
        "Using OpenSSL dynamic link library (DLL) version.<br><br>"
        "%3<br>"
        "License: GnuPG is licensed under GNU GPL-3.0-or-later, libgcrypt under LGPL-2.1-or-later.<br>"
        "This software includes the portable GnuPG executable, sourced from "
        "<a href=\"https://gnupg.org/\">https://gnupg.org/</a>, complying with its original licensing terms.<br><br>"
        "This software itself is licensed under the MIT License.<br><br>"
        "<i>See the LICENSE file in the project for detailed licensing terms.</i>"
    ).arg(QT_VERSION_STR)
    .arg(QString::fromUtf8(OpenSSL_version(OPENSSL_VERSION)))
    .arg(gpgVersionText)
    .arg(QCoreApplication::applicationVersion());

    QLabel *iconLabel = new QLabel(dialog);
    iconLabel->setPixmap(QIcon(":/app.ico").pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *aboutLabel = new QLabel(aboutText, dialog);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);

    layout->addWidget(iconLabel);
    layout->addWidget(aboutLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *licenseMITButton = new QPushButton(tr("MIT 授權"), dialog);
    QPushButton *licenseLGPLButton = new QPushButton(tr("LGPL 授權"), dialog);
    QPushButton *licenseApacheButton = new QPushButton(tr("Apache 授權"), dialog);

    buttonLayout->addWidget(licenseMITButton);
    buttonLayout->addWidget(licenseLGPLButton);
    buttonLayout->addWidget(licenseApacheButton);

    layout->addLayout(buttonLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, dialog);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);

    connect(licenseMITButton, &QPushButton::clicked, this, [=]() {
        showLicenseDialog(tr("MIT 授權條文"), ":/licenses/mit.txt");
    });
    connect(licenseLGPLButton, &QPushButton::clicked, this, [=]() {
        showLicenseDialog(tr("LGPL 授權條文"), ":/licenses/lgpl-3.0.txt");
    });
    connect(licenseApacheButton, &QPushButton::clicked, this, [=]() {
        showLicenseDialog(tr("Apache 授權條文"), ":/licenses/apache-2.0.txt");
    });
    
    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::showLicenseDialog(const QString &title, const QString &resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("錯誤"), tr("無法讀取內嵌的 LICENSE 檔案。"));
        return;
    }

    QTextStream in(&file);
    QString licenseText = in.readAll();
    file.close();

    QDialog *licenseDialog = new QDialog(this);
    licenseDialog->setWindowTitle(title);
    licenseDialog->resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(licenseDialog);
    QTextEdit *textEdit = new QTextEdit(licenseDialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(licenseText);
    layout->addWidget(textEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, licenseDialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, licenseDialog, &QDialog::accept);

    licenseDialog->setLayout(layout);
    licenseDialog->exec();
}

void MainWindow::on_actionAbout_Qt_triggered() {
    QMessageBox::aboutQt(this);
}

// Event filter to handle drag-and-drop for file path input
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->lineEditFilePath)
    {
        if (event->type() == QEvent::DragEnter)
        {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent *>(event);
            if (dragEvent->mimeData()->hasUrls())
            {
                dragEvent->acceptProposedAction();
                return true;
            }
        }
        else if (event->type() == QEvent::Drop)
        {
            QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
            const QList<QUrl> urls = dropEvent->mimeData()->urls();
            if (!urls.isEmpty())
            {
                QString filePath = urls.first().toLocalFile();
                ui->lineEditFilePath->setText(filePath);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
