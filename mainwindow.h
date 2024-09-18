#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "driver.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /* 定义类 */
    Driver *driver = NULL;
    bool hexdisplay_flag = NULL;

private slots:
    void on_quit_btn_clicked();

    void onReadyRead();

    void on_send_btn_released();

    void on_hexdisplay_checkBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    void displayHex();
    void displayText();
};
#endif // MAINWINDOW_H
