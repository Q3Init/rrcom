#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "driver.h"
#include "Platform_Types.h"
#include "crc.h"

#define HEADER_CNT 1
#define ID_CNT 1
#define DLC_CNT 2
#define DATAS_CNT 1
#define CRC_BYTE_CNT 2

#define ACK_OK 0x55
#define ACK_N_OK 0xAA

#define ID_EXTEND_SESSION 0xA3
#define ID_STOP_COMMUNCTION 0xA4

typedef enum
{
    WAIT_HEADER = 0,
    WAIT_ID,
    WAIT_DLC,
    WAIT_DATA,
    WAIT_XOR,
}rx_step;

typedef struct {
    union {
        uint8 val;
        uint8 buf[1];
    } id;
    union {
        uint8 val;
        uint8 buf[2];
    } dlc;
    uint8 datas[8];
    union {
        uint16 val;
        uint8 p_buf[2];
    } Xor;
} InterTpMsgType;

typedef struct
{
    uint8 step;
    uint8 fieldBytesCnt;
    InterTpMsgType msg;
}rxObjType;

typedef enum
{
    OTA_EXTEND_SESSION,
    OTA_STOP_COMMUNCTION,
}Ota_stepType;

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
    CRC *cal_crc = new CRC;
    bool hexdisplay_flag = NULL;
    bool ota_flag = NULL;

private slots:
    /* 数据接收中断函数 */
    void rx_handler();
    /* ota主要接收数据处理函数 */
    void InterRxindication(QByteArray datas);
    /* ota主要发送数据处理函数 */
    void Inter_transmit(uint8 id,uint16 len,uint8 *data);
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
    /* ota功能函数 */
    void ota_mainfunction(InterTpMsgType data);
    /* 扩展会话功能信号 */
    void ota_extend_session();
    /* 停止设备通信功能函数 */
    void ota_stop_communction();

signals:
    /* 数据接收中断信号 */
    void rx_indication(QByteArray rec_buffer);
    /* ota rx信号 */
    void inter_rx_signal(InterTpMsgType data);
    /* 数据发送信号 */
    void inter_tx_signal(uint8 id,uint16 len,uint8 *data);
    /* 扩展会话功能信号 */
    void ota_extend_session_signal();
    /* 停止设备通信功能信号 */
    void ota_stop_communction_signal();

private:
    Ui::MainWindow *ui;
    /* 窗口数据显示hex */
    void displayHex();
    /* 窗口数据显示text */
    void displayText();
};
#endif // MAINWINDOW_H
