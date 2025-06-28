/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label_mode;
    QComboBox *comboBox_mode;
    QLabel *label_input;
    QHBoxLayout *inputFileLayout;
    QLineEdit *lineEdit_input;
    QPushButton *pushButton_browseInput;
    QLabel *label_output;
    QHBoxLayout *outputFileLayout;
    QLineEdit *lineEdit_output;
    QPushButton *pushButton_browseOutput;
    QLabel *label_password;
    QLineEdit *lineEdit_password;
    QPushButton *pushButton_start;
    QTextEdit *textEdit_log;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        verticalLayout = new QVBoxLayout(MainWindow);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label_mode = new QLabel(MainWindow);
        label_mode->setObjectName("label_mode");

        formLayout->setWidget(0, QFormLayout::LabelRole, label_mode);

        comboBox_mode = new QComboBox(MainWindow);
        comboBox_mode->addItem(QString());
        comboBox_mode->addItem(QString());
        comboBox_mode->setObjectName("comboBox_mode");

        formLayout->setWidget(0, QFormLayout::FieldRole, comboBox_mode);

        label_input = new QLabel(MainWindow);
        label_input->setObjectName("label_input");

        formLayout->setWidget(1, QFormLayout::LabelRole, label_input);

        inputFileLayout = new QHBoxLayout();
        inputFileLayout->setObjectName("inputFileLayout");
        lineEdit_input = new QLineEdit(MainWindow);
        lineEdit_input->setObjectName("lineEdit_input");

        inputFileLayout->addWidget(lineEdit_input);

        pushButton_browseInput = new QPushButton(MainWindow);
        pushButton_browseInput->setObjectName("pushButton_browseInput");

        inputFileLayout->addWidget(pushButton_browseInput);


        formLayout->setLayout(1, QFormLayout::FieldRole, inputFileLayout);

        label_output = new QLabel(MainWindow);
        label_output->setObjectName("label_output");

        formLayout->setWidget(2, QFormLayout::LabelRole, label_output);

        outputFileLayout = new QHBoxLayout();
        outputFileLayout->setObjectName("outputFileLayout");
        lineEdit_output = new QLineEdit(MainWindow);
        lineEdit_output->setObjectName("lineEdit_output");

        outputFileLayout->addWidget(lineEdit_output);

        pushButton_browseOutput = new QPushButton(MainWindow);
        pushButton_browseOutput->setObjectName("pushButton_browseOutput");

        outputFileLayout->addWidget(pushButton_browseOutput);


        formLayout->setLayout(2, QFormLayout::FieldRole, outputFileLayout);

        label_password = new QLabel(MainWindow);
        label_password->setObjectName("label_password");

        formLayout->setWidget(3, QFormLayout::LabelRole, label_password);

        lineEdit_password = new QLineEdit(MainWindow);
        lineEdit_password->setObjectName("lineEdit_password");
        lineEdit_password->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(3, QFormLayout::FieldRole, lineEdit_password);


        verticalLayout->addLayout(formLayout);

        pushButton_start = new QPushButton(MainWindow);
        pushButton_start->setObjectName("pushButton_start");

        verticalLayout->addWidget(pushButton_start);

        textEdit_log = new QTextEdit(MainWindow);
        textEdit_log->setObjectName("textEdit_log");
        textEdit_log->setReadOnly(true);

        verticalLayout->addWidget(textEdit_log);


        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "AES Encrypt/Decrypt Tool", nullptr));
        label_mode->setText(QCoreApplication::translate("MainWindow", "Mode:", nullptr));
        comboBox_mode->setItemText(0, QCoreApplication::translate("MainWindow", "Encrypt", nullptr));
        comboBox_mode->setItemText(1, QCoreApplication::translate("MainWindow", "Decrypt", nullptr));

        label_input->setText(QCoreApplication::translate("MainWindow", "Input File:", nullptr));
        pushButton_browseInput->setText(QCoreApplication::translate("MainWindow", "Browse...", nullptr));
        label_output->setText(QCoreApplication::translate("MainWindow", "Output File:", nullptr));
        pushButton_browseOutput->setText(QCoreApplication::translate("MainWindow", "Browse...", nullptr));
        label_password->setText(QCoreApplication::translate("MainWindow", "Password:", nullptr));
        pushButton_start->setText(QCoreApplication::translate("MainWindow", "Start", nullptr));
        textEdit_log->setPlaceholderText(QCoreApplication::translate("MainWindow", "Status messages will appear here...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
