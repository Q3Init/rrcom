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
    Driver *driver = new Driver;
    bool hexdisplay_flag = NULL;
    bool ota_flag = NULL;

private slots:
    /* 数据接收中断函数 */
    void rx_handler();
    /* ota主要接收数据处理函数 */
    void ota_rx_mainfunction(QByteArray datas);
    /* 数据接收窗口 */
    void serial_data_display_window(QByteArray datas);
    /* 发送按钮 */
    void on_send_btn_released();
    /* 16进制显示控件槽函数 */
    void on_hexdisplay_checkBox_toggled(bool checked);
    /* 开始升级槽函数 */
    void on_start_ota_btn_toggled(bool checked);
    /* 退出主菜单 */
    void on_quit_btn_clicked();

signals:
    void rx_indication(QByteArray rec_buffer);

private:
    Ui::MainWindow *ui;
    /* 窗口数据显示hex */
    void displayHex();
    /* 窗口数据显示text */
    void displayText();
};
#endif // MAINWINDOW_H
