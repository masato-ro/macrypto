#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aescrypt.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->pushButton_browseInput, &QPushButton::clicked, this, &MainWindow::browseInput);
    connect(ui->pushButton_browseOutput, &QPushButton::clicked, this, &MainWindow::browseOutput);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onStartClicked() {
    QString input = ui->lineEdit_input->text();
    QString output = ui->lineEdit_output->text();
    QString password = ui->lineEdit_password->text();
    QString mode = ui->comboBox_mode->currentText();

    AESCrypt crypt;
    bool ok = false;

    if (mode == "Encrypt") {
        ok = crypt.encryptFile(input, output, password);
    } else {
        ok = crypt.decryptFile(input, output, password);
    }

    QString result = ok ? "成功" : "失敗";
    ui->textEdit_log->append(QString("%1 %2").arg(mode, result));
}

void MainWindow::browseInput() {
    QString filename = QFileDialog::getOpenFileName(this, "Select input file");
    if (!filename.isEmpty()) {
        ui->lineEdit_input->setText(filename);
    }
}

void MainWindow::browseOutput() {
    QString filename = QFileDialog::getSaveFileName(this, "Select output file");
    if (!filename.isEmpty()) {
        ui->lineEdit_output->setText(filename);
    }
}
