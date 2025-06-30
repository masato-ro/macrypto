#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:  
    void on_pushButton_start_clicked();
    void on_pushButton_browseInput_clicked();
    void on_pushButton_browseOutput_clicked();
    void on_btnHashText_clicked();
    void on_btnSelectFile_clicked();
    void on_btnHashFile_clicked();
    void on_pushButton_copyHash_clicked();
    void on_pushButtonGPG_browseInput_clicked();
    void on_pushButtonGPG_browseOutput_clicked();
    void on_pushButton_decryptGPG_clicked();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

private:
    Ui::MainWindow *ui;
    void showLicenseDialog(const QString &title, const QString &resourcePath);
};

#endif // MAINWINDOW_H
