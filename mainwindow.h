#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "driver.h"
#include "Platform_Types.h"
#include "crc.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QTimer>

#define HEADER_CNT 1
#define ID_CNT 1
#define DLC_CNT 2
#define DATAS_CNT 1
#define CRC_BYTE_CNT 2

#define ACK_OK 0x55
#define ACK_N_OK 0xAA

#define ID_EXTEND_SESSION       0xA3
#define ID_STOP_COMMUNCTION     0xA4
#define ID_PROGRAMMING_SESSION  0xA2
#define ID_REQUEST_ERASE        0xA5
#define ID_REQUEST_DOWNLOAD     0x06
#define ID_DATA_TRANSMISSION    0xA7
#define ID_TRANSMISSION_EXIT    0xA8
#define ID_CHECK_APP_INTEGRITY  0xA9
#define ID_SOFTWARE_RESET       0xAA
#define ID_START_COMMUNCTION    0xAB
#define ID_SESSION_PERSISTENCE  0x3E


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
        uint16 val;
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
    OTA_EXTEND_SESSION = 0,
    OTA_STOP_COMMUNCTION,
    OTA_PROGRAMMING_SEESION,
    OTA_REQUEST_ERASE,
    OTA_REQUEST_DOWNLOAD,
    OTA_DATA_TRANSMISSION,
    OTA_TRANSMISSION_EXIT,
    OTA_CHECK_APP_INTEGRITY,
    OTA_SOFTWARE_RESET,
    OTA_START_COMMUNCTION,
    OTA_SESSION_PERSISTENCE,
}Ota_stepType;

typedef struct
{
    union {
        uint32 val;
        uint8 buf[4];
    }address;
}addressType;

typedef struct
{
    union {
        uint16 val;
        uint8 buf[2];
    }app_block_cnt;
}APP_block_cnt_Type;

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
    QString fileName = 0;

    uint32 bcd_to_hex(uint32 bcd_data);
    /* Qstring字符串数据  直接转为 16进制数 */
    uint8 QString_to_uint8_buffer(QString *str,uint8 *data);

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
    /* 编程会话功能函数 */
    void ota_programming_session();
    /* 请求擦除功能函数 */
    void ota_request_erase();
    /* 请求下载功能函数 */
    void ota_request_download();
    /* 数据传输功能函数 */
    void ota_data_transmission();
    /* 传输退出功能函数 */
    void ota_transmission_exit();
    /* 校验APP完整性功能函数 */
    void ota_check_app_integrity();
    /* 软件复位功能函数 */
    void ota_software_reset();
    /* 开始通信功能函数 */
    void ota_start_communction();
    /* 会话保持功能函数 */
    void ota_session_presistence();
    /* 打开升级文件hex槽函数 */
    void on_open_download_file_btn_released();

signals:
    /* 数据接收中断信号 */
    void rx_indication_signal(QByteArray rec_buffer);
    /* ota rx信号 */
    void inter_rx_signal(InterTpMsgType data);
    /* 数据发送信号 */
    void inter_tx_signal(uint8 id,uint16 len,uint8 *data);
    /* 扩展会话功能信号 */
    void ota_extend_session_signal();
    /* 停止设备通信功能信号 */
    void ota_stop_communction_signal();
    /* 编程会话功能信号 */
    void ota_programming_session_signal();
    /* 请求擦除功能信号 */
    void ota_request_erase_signal();
    /* 请求下载功能信号 */
    void ota_request_download_signal();
    /* 数据传输功能信号 */
    void ota_data_transmission_signal();
    /* 传输退出功能信号 */
    void ota_transmission_exit_signal();
    /* 校验APP完整性功能信号 */
    void ota_check_app_integrity_signal();
    /* 软件复位功能信号 */
    void ota_software_reset_signal();
    /* 开始通信功能信号 */
    void ota_start_communction_signal();
    /* 会话保持功能信号 */
    void ota_session_presistence_signal();

private:
    Ui::MainWindow *ui;
    /* 窗口数据显示hex */
    void displayHex();
    /* 窗口数据显示text */
    void displayText();
};
#endif // MAINWINDOW_H
