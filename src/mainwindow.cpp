#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aescrypt.h"
#include "aescontroller.h"
#include "keygen.h"
#include "keycontroller.h"
#include "hashutil.h"
#include "hashcontroller.h"
#include "gpgcontroller.h"

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
    ui->comboBoxKeyLength->addItems(QStringList() << "1024" << "2048" << "4096");
    ui->lineEditFilePath->installEventFilter(this);
    ui->lineEditFilePath->setAcceptDrops(true);

    connect(ui->checkBoxConvertToSSH, &QCheckBox::toggled, this, [this](bool checked) {
        ui->lineEditComment->setEnabled(checked);
    });

    connect(ui->pushButtonCopyKey, &QPushButton::clicked, this, &MainWindow::copyOpenSSHPublicKeyToClipboard);

    // 設定 tab 切換事件
    connect(ui->tabGenerateKey, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // 初始時設定當前 tab
    onTabChanged(ui->tabGenerateKey->currentIndex());

    // 設定動作與對應的 tab index
    connect(ui->actionAES_Encrypt_Decrypt, &QAction::triggered, this, [=]() {
        ui->tabGenerateKey->setCurrentIndex(0);
    });
    connect(ui->actionGenerate_Key, &QAction::triggered, this, [=]() {
        ui->tabGenerateKey->setCurrentIndex(1);
    });
    connect(ui->actionHash_Digest, &QAction::triggered, this, [=]() {
        ui->tabGenerateKey->setCurrentIndex(2);
    });
    connect(ui->actionGPG_Decrypt, &QAction::triggered, this, [=]() {
        ui->tabGenerateKey->setCurrentIndex(3);
    });

    // 初始化 GPG 控制器
    gpgController = new GPGController(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onTabChanged(int index) {
    ui->actionAES_Encrypt_Decrypt->setChecked(index == 0);
    ui->actionGenerate_Key->setChecked(index == 1);
    ui->actionHash_Digest->setChecked(index == 2);
    ui->actionGPG_Decrypt->setChecked(index == 3);
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

    bool ok = false;
    if (mode == "Encrypt") {
        ui->textEdit_log->append("🔐 進行加密中...");
        ok = AESController::runEncrypt(input, output, password, [=](int percent) {
            ui->progressBar->setValue(percent);
            QCoreApplication::processEvents();
        });
    } else {
        ui->textEdit_log->append("🔓 進行解密中...");
        ok = AESController::runDecrypt(input, output, password, [=](int percent) {
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

void MainWindow::on_pushButtonClearLog_clicked() {
    ui->textEdit_log->clear();
}



void MainWindow::on_pushButtonBrowsePrivate_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("選擇私鑰輸出位置"), "", tr("PEM 檔案 (*.pem)"));
    if (!fileName.isEmpty()) {
        ui->lineEditPrivateKeyPath->setText(fileName);
    }
}

void MainWindow::on_pushButtonBrowsePublic_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("選擇公鑰輸出位置"), "", tr("PEM 檔案 (*.pem)"));
    if (!fileName.isEmpty()) {
        ui->lineEditPublicKeyPath->setText(fileName);
    }
}

void MainWindow::on_pushButtonGenerateKey_clicked() {
    int bits = ui->comboBoxKeyLength->currentText().toInt();
    QString privPath = ui->lineEditPrivateKeyPath->text();
    QString pubPath = ui->lineEditPublicKeyPath->text();

    if (privPath.isEmpty() || pubPath.isEmpty()) {
        ui->textEditKeyLog->append("❗ 請先設定私鑰和公鑰輸出路徑！");
        return;
    }

    ui->progressBarKey->setRange(0, 1);
    ui->progressBarKey->setValue(1);

    KeyController controller;
    bool success = controller.generate(bits, privPath, pubPath,
                                       ui->checkBoxConvertToSSH->isChecked(),
                                       ui->lineEditComment->text());

    if (!success) {
        ui->textEditKeyLog->append("❌ 金鑰產生失敗：" + controller.getLastError());
        return;
    }

    ui->textEditKeyLog->append("✅ 金鑰產生完成！");
    ui->textEditKeyLog->append("🔐 私鑰：" + privPath);
    ui->textEditKeyLog->append("🔓 公鑰：" + pubPath);
    ui->textEditKeyLog->append("⚠️ 請妥善保管私鑰，切勿洩漏或上傳！");

    if (ui->checkBoxConvertToSSH->isChecked()) {
        ui->textEditKeyLog->append("🔑 OpenSSH 公鑰已儲存：" + controller.getSSHOutputPath());
        ui->textEditKeyLog->append("📄 OpenSSH 公鑰內容如下：");
        ui->textEditKeyLog->append("----------- BEGIN SSH PUB KEY -----------");
        ui->textEditKeyLog->append(controller.getSSHPublicKey());
        ui->textEditKeyLog->append("------------ END SSH PUB KEY ------------");
    }
}

void MainWindow::copyOpenSSHPublicKeyToClipboard()
{
    QString pubKeyPath;

    // 嘗試從最後產生的公鑰檔路徑取得
    QString logText = ui->textEditKeyLog->toPlainText();
    QString marker = "🔑 OpenSSH 公鑰已儲存：";
    int pos = logText.lastIndexOf(marker);
    if (pos != -1) {
        int pathStart = pos + marker.length();
        int pathEnd = logText.indexOf('\n', pathStart);
        pubKeyPath = logText.mid(pathStart, pathEnd - pathStart).trimmed();
    }

    if (QFile::exists(pubKeyPath)) {
        QFile file(pubKeyPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString pubKeyText = QString::fromUtf8(file.readAll());
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(pubKeyText);
            ui->textEditKeyLog->append("📋 OpenSSH 公鑰已複製到剪貼簿！");
            file.close();
        } else {
            ui->textEditKeyLog->append("❌ 無法開啟 OpenSSH 公鑰檔案！");
        }
    } else {
        ui->textEditKeyLog->append("❌ 找不到 OpenSSH 公鑰路徑！");
    }
}

void MainWindow::on_pushButtonClearEditKeyLog_clicked() {
    ui->textEditKeyLog->clear();
}



void MainWindow::on_btnSelectFile_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("選擇檔案"));
    if (!fileName.isEmpty()) {
        ui->lineEditFilePath->setText(fileName);
    }
}

HashAlgorithm MainWindow::currentSelectedAlgorithm() const {
    QString algorithmStr = ui->comboAlgorithm->currentText();
    return toHashAlgorithm(algorithmStr);
}

void MainWindow::on_btnHashText_clicked() {
    HashAlgorithm algorithm = currentSelectedAlgorithm();
    QString inputText = ui->plainTextInput->toPlainText();

    if (inputText.trimmed().isEmpty()) {
        ui->textEditHashResult->setPlainText("❗ 請輸入文字以計算雜湊值。");
        return;
    }
    
    QString result = HashController::runHashFromText(inputText, algorithm);
    
    if (result.isEmpty()) {
        ui->textEditHashResult->setPlainText("❌ 雜湊失敗，請檢查輸入內容。");
    } else {
        ui->textEditHashResult->setPlainText(result);
    }
}

void MainWindow::on_btnHashFile_clicked() {
    HashAlgorithm algorithm = currentSelectedAlgorithm();
    QString fileName = ui->lineEditFilePath->text();

    if (fileName.isEmpty()) {
        ui->textEditHashResult->setPlainText("❗ 請選擇要計算雜湊值的檔案。");
        return;
    }

    ui->progressBar_2->setValue(0);

    QString result = HashController::runHashFromFile(fileName, algorithm, [&](int percent){
        ui->progressBar_2->setValue(percent);
        QCoreApplication::processEvents();  // 確保 UI 即時更新
    });

    if (result.isEmpty()) {
        ui->textEditHashResult->setPlainText("❌ 雜湊失敗，可能無法讀取檔案。");
    } else {
        ui->textEditHashResult->setPlainText(result);
    }
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

void MainWindow::on_pushButtonClearHashResult_clicked() {
    ui->lineEditFilePath->clear();
    ui->plainTextInput->clear();
    ui->textEditHashResult->clear();
    ui->progressBar_2->setValue(0);
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

// 這是你的 MainWindow.cpp 中的 on_pushButton_decryptGPG_clicked 函式

void MainWindow::on_pushButton_decryptGPG_clicked() {
    // 1. 獲取輸入/輸出路徑並初始化 UI 狀態
    QString input = ui->lineEditGPG_input->text();
    QString output = ui->lineEditGPG_output->text();
    ui->textEditGPG_log->clear(); // 清空日誌區域
    
    // 將進度條設定為初始狀態：0% 且不顯示文字
    ui->progressBarGPG->setRange(0, 100); 
    ui->progressBarGPG->setValue(0);
    ui->progressBarGPG->setTextVisible(false); // 初始狀態不顯示百分比文字

    // 禁用按鈕，防止重複點擊
    ui->pushButton_decryptGPG->setEnabled(false); 

    // 2. 輸入驗證
    if (input.isEmpty() || output.isEmpty()) {
        ui->textEditGPG_log->append("❗請填入輸入與輸出路徑！");
        
        // 恢復按鈕和進度條
        ui->pushButton_decryptGPG->setEnabled(true); 
        ui->progressBarGPG->setRange(0, 100);
        ui->progressBarGPG->setValue(0);
        ui->progressBarGPG->setTextVisible(true); // 顯示百分比文字
        return;
    }

    // 3. 呼叫 decryptFile 並傳遞回呼函式
    gpgController->decryptFile(input, output,
        // logCallback: 接收 GPGController 發送的日誌訊息並顯示
        [=](const QString &logMsg) {
            ui->textEditGPG_log->append(logMsg);
        },
        // progressCallback: 接收進度更新 (0 或 100) 並控制進度條顯示
        [=](int percent) {
            if (percent == 0) {
                // 操作開始：設定為不確定模式 (會來回跑動的動畫)
                ui->progressBarGPG->setRange(0, 0); // 這是關鍵！將 min 和 max 都設為 0
                ui->progressBarGPG->setValue(0);    // 值不重要，模式已設定
                ui->progressBarGPG->setTextVisible(false); // 不顯示百分比文字
            } else if (percent == 100) {
                // 操作結束：設定回確定模式，並顯示 100%
                ui->progressBarGPG->setRange(0, 100); // 恢復正常範圍
                ui->progressBarGPG->setValue(100);    // 設定為 100%
                ui->progressBarGPG->setTextVisible(true); // 顯示百分比文字
                // 這裡的狀態文字將由 finishedCallback 最終決定
            }
        },
        // finishedCallback: 接收最終解密結果 (成功或失敗) 並更新 UI
        [=](bool success) {
            // progressCallback(100) 已經處理了進度條到 100%
            ui->pushButton_decryptGPG->setEnabled(true); // 重新啟用按鈕
        }
    );
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
    iconLabel->setPixmap(QIcon(":/icons/app.ico").pixmap(64, 64));
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
    QPushButton *licenseGPLButton = new QPushButton(tr("GPL 授權"), dialog);

    buttonLayout->addWidget(licenseMITButton);
    buttonLayout->addWidget(licenseLGPLButton);
    buttonLayout->addWidget(licenseApacheButton);
    buttonLayout->addWidget(licenseGPLButton);

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
    connect(licenseGPLButton, &QPushButton::clicked, this, [=]() {
        showLicenseDialog(tr("GPL 授權條文"), ":/licenses/gpl-3.0.txt");
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