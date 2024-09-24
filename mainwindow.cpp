#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Platform_Types.h"

static const uint8 HeaderPattern[HEADER_CNT] = {0XEE};
static Ota_stepType ota_step = OTA_EXTEND_SESSION;
static APP_block_cnt_Type APP1 = {0};
static qint64 file_seek_cnt = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->driver->show();
    /* 收到driver界面的连接成功信号 */
    connect(this->driver,&Driver::driver_ui_link,this,[this]{
        this->driver->hide();
        this->show();
    });
    /* sender：串口, signal: 串口readyRead, receiver:this, member: rxhandler */
    connect(&this->driver->serialPort,&QSerialPort::readyRead,this,&MainWindow::rx_handler);
    /* sender：this, signal: rx_indication_signal, receiver:this, member: serial_data_display_window */
    connect(this,&MainWindow::rx_indication_signal,this,&MainWindow::serial_data_display_window);
    /* sender：this, signal: rx_indication_signal, receiver:this, member: InterRxindication */
    connect(this,&MainWindow::rx_indication_signal,this,&MainWindow::InterRxindication);
    /* sender：this, signal: inter_rx_signal, receiver:this, member: ota_mainfunction */
    connect(this,&MainWindow::inter_rx_signal,this,&MainWindow::ota_mainfunction);
    /* sender：this, signal: inter_tx_signal, receiver:this, member: Inter_transmit */
    connect(this,&MainWindow::inter_tx_signal,this,&MainWindow::Inter_transmit);
    /* sender：this, signal: ota_extend_session_signal, receiver:this, member: ota_extend_session */
    connect(this,&MainWindow::ota_extend_session_signal,this,&MainWindow::ota_extend_session);
    /* sender：this, signal: ota_stop_communction_signal, receiver:this, member: ota_stop_communction */
    connect(this,&MainWindow::ota_stop_communction_signal,this,&MainWindow::ota_stop_communction);
    /* sender：this, signal: ota_programming_session_signal, receiver:this, member: ota_programming_session */
    connect(this,&MainWindow::ota_programming_session_signal,this,&MainWindow::ota_programming_session);
    /* sender：this, signal: ota_request_erase_signal, receiver:this, member: ota_request_erase */
    connect(this,&MainWindow::ota_request_erase_signal,this,&MainWindow::ota_request_erase);
    /* sender：this, signal: ota_request_download_signal, receiver:this, member: ota_request_download */
    connect(this,&MainWindow::ota_request_download_signal,this,&MainWindow::ota_request_download);
    /* sender：this, signal: ota_data_transmission_signal, receiver:this, member: ota_data_transmission */
    connect(this,&MainWindow::ota_data_transmission_signal,this,&MainWindow::ota_data_transmission);
    /* sender：this, signal: ota_transmission_exit_signal, receiver:this, member: ota_transmission_exit */
    connect(this,&MainWindow::ota_transmission_exit_signal,this,&MainWindow::ota_transmission_exit);
    /* sender：this, signal: ota_check_app_integrity_signal, receiver:this, member: ota_check_app_integrity */
    connect(this,&MainWindow::ota_check_app_integrity_signal,this,&MainWindow::ota_check_app_integrity);
    /* sender：this, signal: ota_software_reset_signal, receiver:this, member: ota_software_reset */
    connect(this,&MainWindow::ota_software_reset_signal,this,&MainWindow::ota_software_reset);
    /* sender：this, signal: ota_start_communction_signal, receiver:this, member: ota_start_communction */
    connect(this,&MainWindow::ota_start_communction_signal,this,&MainWindow::ota_start_communction);
    /* sender：this, signal: ota_session_presistence_signal, receiver:this, member: ota_session_presistence */
    connect(this,&MainWindow::ota_session_presistence_signal,this,&MainWindow::ota_session_presistence);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* 数据接收中断函数 */
void MainWindow::rx_handler()
{
    qDebug() << "[rx_handler]"<< Qt::endl;
    QByteArray recv_datas = this->driver->serialPort.readAll();
    emit this->rx_indication_signal(recv_datas);
}

/* ota主要接收数据处理函数 */
void MainWindow::InterRxindication(QByteArray datas)
{
    qDebug() << "[InterRxindication]"<< Qt::endl;
    rxObjType *objPtr;
    uint8 rx_buffer[64] = {0};
    uint8 data;
    /* 升级接收处理 */
    if (this->ota_flag == true)
    {
        /* 取数据 */
        for (int i = 0;i < datas.size(); i++)
        {
            rx_buffer[i] = datas[i];
            qDebug() << rx_buffer[i];
        }
        qDebug() << Qt::endl;

        for (int index = 0; index < datas.size(); index ++)
        {
            data = rx_buffer[index];
            switch(objPtr->step)
            {
            case WAIT_HEADER:
                if (data == HeaderPattern[objPtr->fieldBytesCnt])
                {
                    objPtr->fieldBytesCnt++;
                    if (objPtr->fieldBytesCnt == HEADER_CNT)
                    {
                        objPtr->msg.id.val = 0;
                        objPtr->fieldBytesCnt = 0;
                        objPtr->step = WAIT_ID;
                    }
                }
                break;
            case WAIT_ID:
                objPtr->msg.id.buf[objPtr->fieldBytesCnt] = data;
                objPtr->fieldBytesCnt++;
                if (objPtr->fieldBytesCnt == ID_CNT)
                {
                    objPtr->fieldBytesCnt = 0;
                    objPtr->step = WAIT_DLC;
                }
                break;
            case WAIT_DLC:
                objPtr->msg.dlc.buf[objPtr->fieldBytesCnt] = data;
                objPtr->fieldBytesCnt++;
                if (objPtr->fieldBytesCnt == DLC_CNT)
                {
                    objPtr->fieldBytesCnt = 0;
                    objPtr->step = WAIT_DATA;
                }
                break;
            case WAIT_DATA:
                objPtr->msg.datas[objPtr->fieldBytesCnt] = data;
                objPtr->fieldBytesCnt++;
                if (objPtr->fieldBytesCnt == DATAS_CNT) {
                    objPtr->fieldBytesCnt = 0;
                    objPtr->step = WAIT_XOR;
                }
                break;
            case WAIT_XOR:
                //CRC对了之后，判断下功能码，然后emit信号到对应功能槽函数
                objPtr->msg.Xor.p_buf[objPtr->fieldBytesCnt] = data;
                objPtr->fieldBytesCnt++;
                if (objPtr->fieldBytesCnt == CRC_BYTE_CNT)
                {
                    if (objPtr->msg.Xor.val == this->cal_crc->apu_CRC16(rx_buffer,(datas.size() - 2)))
                    {
                        emit this->inter_rx_signal(objPtr->msg);
                    }
                    objPtr->fieldBytesCnt = 0;
                    objPtr->step = WAIT_HEADER;
                }
                break;
            default:
                objPtr->fieldBytesCnt = 0;
                objPtr->step = WAIT_HEADER;
                break;
            }
        }
    }
    memset(rx_buffer,0,sizeof(rx_buffer));
}

/* ota主要发送数据处理函数 */
void MainWindow::Inter_transmit(uint8 id,uint16 len,uint8 *data)
{
    qDebug() << "[Inter_transmit]"<< Qt::endl;
    uint8 interTpTransmitMsgBuf[256] = {0};
    uint16 crc = 0;
    QByteArray tx_datas;
    interTpTransmitMsgBuf[0] = HeaderPattern[0];
    interTpTransmitMsgBuf[1] = id;
    interTpTransmitMsgBuf[2] = (uint8)(len >> 8);
    interTpTransmitMsgBuf[3] = (uint8)(len);
    memcpy(interTpTransmitMsgBuf+4,data,len);
    crc = this->cal_crc->apu_CRC16(interTpTransmitMsgBuf,len + 4);
    interTpTransmitMsgBuf[len + 4] = (uint8)(crc >> 8);
    interTpTransmitMsgBuf[len + 5] = (uint8)(crc);
    tx_datas.resize(len + 6);
    memcpy(tx_datas.data(),&interTpTransmitMsgBuf,len + 6);
    if (this->driver->serialPort.write(tx_datas) > 0) {
        qDebug() << "intertp tx tx_datas:" << tx_datas.toHex() << Qt::endl;
    } else {
        qDebug() << "intertp tx fail" << Qt::endl;
    }
}

/* 数据接收窗口 */
void MainWindow::serial_data_display_window(QByteArray datas)
{
    qDebug() << "[serial_data_display_window]"<< Qt::endl;
    QByteArray recBuf = datas;
    QString str_rev;
    if (this->hexdisplay_flag == false)
    {
        if(ui->chk_rev_time_checkBox->checkState() == Qt::Checked){
            QDateTime nowtime = QDateTime::currentDateTime();
            str_rev = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] " + "RX: ";
            str_rev += QString(recBuf).append("\r\n");
        }
        else{
            // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
            //ui->recvEdit->appendPlainText(buf);

            if(ui->chk_rev_line_checkBox->checkState() == Qt::Checked){
                str_rev = QString(recBuf).append("\r\n");
            }
            else
            {
                str_rev = QString(recBuf);
            }
        }
    }
    else
    {
        // 16进制显示，并转换为大写
        QString str1 = recBuf.toHex().toUpper().data();//.data();
        // 添加空格
        QString str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid(i,2);
            str2 += " ";
        }
        if(ui->chk_rev_time_checkBox->checkState() == Qt::Checked)
        {
            QDateTime nowtime = QDateTime::currentDateTime();
            str_rev = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] " + "RX: ";
            str_rev += str2.append("\r\n");
        }
        else
        {
            if(ui->chk_rev_line_checkBox->checkState() == Qt::Checked)
                str_rev += str2.append("\r\n");
            else
                str_rev = str2;

        }
    }
    ui->recv_edit->insertPlainText(str_rev);
    ui->recv_edit->moveCursor(QTextCursor::End);
}

/* 发送按钮 */
void MainWindow::on_send_btn_released()
{
    qDebug() << "[on_send_btn_released]"<< Qt::endl;
    QByteArray array;
    auto dataStr = ui->send_edit->toPlainText();
    //Hex复选框
    if(ui->hexdisplay_checkBox->checkState() == Qt::Checked){
        //array = QString2Hex(data);  //HEX 16进制
        array = QByteArray::fromHex(dataStr.toLatin1().data());
    }else{
        //array = data.toLatin1();    //ASCII
        array = dataStr.toLocal8Bit().data();
    }

    if(ui->sendnewline_checkBox->checkState() == Qt::Checked){
        array.append("\r\n");
    }
    //显示到接收框

    //发送成功
    if (this->driver->serialPort.write(array) > 0) {
        qDebug() << "send ok" << Qt::endl;
    } else {
        qDebug() << "send fail" << Qt::endl;
    }
}

/* 16进制显示控件槽函数 */
void MainWindow::on_hexdisplay_checkBox_toggled(bool checked)
{
    qDebug() << "[on_hexdisplay_checkBox_toggled]"<< Qt::endl;
    if(checked)
    {
        // displayHex();
        this->hexdisplay_flag = true;
    }
    else
    {
        // displayText();
        this->hexdisplay_flag = false;
    }
}

/* 开始升级槽函数 */
void MainWindow::on_start_ota_btn_toggled(bool checked)
{
    qDebug() << "[on_start_ota_btn_toggled]"<< Qt::endl;
    if(checked)
    {
        this->ota_flag = true;
        emit this->ota_extend_session_signal();
        ui->start_ota_btn->setText("停止升级");
    }
    else
    {
        this->ota_flag = false;
        ui->start_ota_btn->setText("开始升级");
    }
    qDebug() << "on_start_ota_btn_toggled, ota: "<<this->ota_flag << Qt::endl;
}

/* 退出主菜单 */
void MainWindow::on_quit_btn_clicked()
{
    qDebug() << "[on_quit_btn_clicked]"<< Qt::endl;
    this->hide();
    this->driver->show();
    /* 退出主菜单并关闭驱动 */
    this->driver->serialPort.close();
}

/* 窗口数据显示hex */
void MainWindow::displayHex()
{
    qDebug() << "[displayHex]"<< Qt::endl;
    //先把数据拿出来
    auto dataStr = ui->recv_edit->toPlainText();
    //转成16进制
    auto hexData = dataStr.toLocal8Bit().toHex().toUpper();
    //写回去
    ui->recv_edit->setPlainText(hexData);
}

/* 窗口数据显示text */
void MainWindow::displayText()
{
    qDebug() << "[displayText]"<< Qt::endl;
    //先把数据拿出来
    auto dataStr = ui->recv_edit->toPlainText();
    //转成文本
    auto textData = QString::fromLocal8Bit(dataStr.toLocal8Bit());
    //写回去
    ui->recv_edit->setPlainText(textData);
}

/* ota功能函数 */
void MainWindow::ota_mainfunction(InterTpMsgType datas)
{
    qDebug() << "[ota_mainfunction]"<< Qt::endl;
    InterTpMsgType info = datas;
    switch(ota_step)
    {
    case OTA_EXTEND_SESSION:
        if (info.id.val == ID_EXTEND_SESSION)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_stop_communction_signal();
                ota_step = OTA_STOP_COMMUNCTION;
            }
            /* 收到扩展会话应答，触发停止通信服务信号 */
        }
        break;
    case OTA_STOP_COMMUNCTION:
        if (info.id.val == ID_STOP_COMMUNCTION)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_programming_session_signal();
                ota_step = OTA_PROGRAMMING_SEESION;
            }
        }
        break;
    case OTA_PROGRAMMING_SEESION:
        if (info.id.val == ID_PROGRAMMING_SESSION)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_request_erase_signal();
                ota_step = OTA_REQUEST_ERASE;
            }
        }
        break;
    case OTA_REQUEST_ERASE:
        if (info.id.val == ID_REQUEST_ERASE)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_request_download_signal();
                ota_step = OTA_REQUEST_DOWNLOAD;
            }
        }
        break;
    case OTA_REQUEST_DOWNLOAD:
        if (info.id.val == ID_REQUEST_DOWNLOAD)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_data_transmission_signal();
                ota_step = OTA_DATA_TRANSMISSION;
            }
        }
        break;
    case OTA_DATA_TRANSMISSION:
        if (info.id.val == ID_DATA_TRANSMISSION)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_transmission_exit_signal();
                ota_step = OTA_TRANSMISSION_EXIT;
            }
        }
        break;
    case OTA_TRANSMISSION_EXIT:
        if (info.id.val == ID_TRANSMISSION_EXIT)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_check_app_integrity_signal();
                ota_step = OTA_CHECK_APP_INTEGRITY;
            }
        }
        break;
    case OTA_CHECK_APP_INTEGRITY:
        if (info.id.val == ID_CHECK_APP_INTEGRITY)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_software_reset_signal();
                ota_step = OTA_SOFTWARE_RESET;
            }
        }
        break;
    case OTA_SOFTWARE_RESET:
        if (info.id.val == ID_SOFTWARE_RESET)
        {
            if(info.datas[0] == ACK_OK)
            {
                emit this->ota_start_communction_signal();
                ota_step = OTA_SOFTWARE_RESET;
            }
        }
        break;
    case OTA_START_COMMUNCTION:
        if (info.id.val == ID_START_COMMUNCTION)
        {
            if(info.datas[0] == ACK_OK)
            {
                ota_step = OTA_EXTEND_SESSION;
            }
        }
        break;
    default:
        break;
    }
}

/* 扩展会话功能函数 */
void MainWindow::ota_extend_session()
{
    qDebug() << "[ota_extend_session]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 1;
    emit this->inter_tx_signal(ID_EXTEND_SESSION ,len ,&data);
}

/* 停止设备通信功能函数 */
void MainWindow::ota_stop_communction()
{
    qDebug() << "[ota_stop_communction_signal]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 2;
    emit this->inter_tx_signal(ID_STOP_COMMUNCTION ,len ,&data);
}

/* 编程会话功能函数 */
void MainWindow::ota_programming_session()
{
    qDebug() << "[ota_programming_session]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_PROGRAMMING_SESSION ,len ,&data);
}

/* 请求擦除功能函数 */
void MainWindow::ota_request_erase()
{
    qDebug() << "[ota_request_erase]"<< Qt::endl;
    QString line = 0;
    QString hexdata = 0;
    addressType hex_address = {0};
    QFile file(this->fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        file.seek(0);
        line = file.readLine();
        hexdata = line.mid(9, line.length() - 13);
        hex_address.address.val = (this->bcd_to_hex((uint32)hexdata.toUInt()) << 16);
    }
    file.close();
    emit this->inter_tx_signal(ID_REQUEST_ERASE ,sizeof(hex_address.address.val) ,hex_address.address.buf);
}

/* 请求下载功能函数 */
void MainWindow::ota_request_download()
{
    qDebug() << "[ota_request_download]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_REQUEST_ERASE ,len ,&data);
}

/* 数据传输功能函数 */
void MainWindow::ota_data_transmission()
{
    qDebug() << "[ota_data_transmission]"<< Qt::endl;

    uint8 tx_buffer[300];
    uint16 tx_buffer_index =0;
    /* 读取hex文件参数 */
    QString line = 0;
    QString recordType = 0;
    QString hexdata = 0;
    QFile file(this->fileName);
    APP1.app_block_cnt.val++;
    memcpy(tx_buffer,APP1.app_block_cnt.buf,sizeof(APP1.app_block_cnt.val));
    tx_buffer_index +=2;
    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        for(int i = 0; i < 16; i++) {
            file.seek(file_seek_cnt);
            line = file.readLine();
            recordType = line.mid(7,2);
            hexdata = line.mid(9, line.length() - 13);
            if (recordType == "0x00") {
                if (line.length() < 45) {
                    this->QString_to_uint8_buffer(&hexdata,(tx_buffer + tx_buffer_index));
                    file_seek_cnt+= line.length();
                    tx_buffer_index += (hexdata.size() /2);
                } else {
                    this->QString_to_uint8_buffer(&hexdata,(tx_buffer + tx_buffer_index));
                    file_seek_cnt += line.length();
                    tx_buffer_index += ((hexdata.size() /2) + (hexdata.size() % 2));
                    break;
                }
            } else if (recordType == "0x01") {
                file_seek_cnt = 0;
            } else {
                file_seek_cnt += line.length();
            }
        }
        file.close();
        emit this->inter_tx_signal(ID_DATA_TRANSMISSION,tx_buffer_index,tx_buffer);
        memset(tx_buffer,0,sizeof(tx_buffer));
    }
}

/* 传输退出功能函数 */
void MainWindow::ota_transmission_exit()
{
    qDebug() << "[ota_transmission_exit]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_TRANSMISSION_EXIT ,len ,&data);
}

/* 校验APP完整性功能函数 */
void MainWindow::ota_check_app_integrity()
{
    qDebug() << "[ota_check_app_integrity]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_TRANSMISSION_EXIT ,len ,&data);
}

/* 软件复位功能函数 */
void MainWindow::ota_software_reset()
{
    qDebug() << "[ota_software_reset]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_SOFTWARE_RESET ,len ,&data);
}

/* 开始通信功能函数 */
void MainWindow::ota_start_communction()
{
    qDebug() << "[ota_start_communction]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 3;
    emit this->inter_tx_signal(ID_START_COMMUNCTION ,len ,&data);
}

/* 软件复位功能函数 */
void MainWindow::ota_session_presistence()
{
    qDebug() << "[ota_session_presistence]"<< Qt::endl;
    uint16 len = 1;
    uint8 data = 0;
    emit this->inter_tx_signal(ID_SESSION_PERSISTENCE ,len ,&data);
}

/* 打开升级文件hex槽函数 */
void MainWindow::on_open_download_file_btn_released()
{
    this->fileName = QFileDialog::getOpenFileName(this,tr("Open File"),QDir::currentPath(), tr("*.hex"));
    ui->download_file_name_edit->setText(this->fileName);
}

/* Qstring字符串数据  直接转为 16进制数 */
uint8 MainWindow::QString_to_uint8_buffer(QString *str,uint8 *data)
{
    uint8 ret = E_NOK;
#if 0
    for(int i =0; i<(str->size()); i+=2)
    {
        data[i/2] = str->mid(i,2).toUInt(nullptr,16);
    }
    ret = E_OK;
#else
    QByteArray byte = QByteArray::fromHex(str->toUtf8());
    int i = 0;
    for(quint8 s_buf : byte)
    {
        data[i++] = s_buf;
    }
    ret = E_OK;
#endif
    return ret;
}

uint32 MainWindow::bcd_to_hex(uint32 bcd_data)
{
    uint32 ret = 0;
    if(bcd_data < 100)
    {
        ret = (uint32)(((bcd_data / 10) << 4) + bcd_data % 10);
    } else if (bcd_data < 1000)
    {
        ret = (uint32)(((bcd_data / 100) << 8) + bcd_data % 100);
    } else if (bcd_data < 10000)
    {
        ret = (uint32)(((bcd_data / 1000) << 12) + bcd_data % 1000);
    } else if (bcd_data < 100000)
    {
        ret = (uint32)(((bcd_data / 10000) << 16) + bcd_data % 10000);
    } else if (bcd_data < 1000000)
    {
        ret = (uint32)(((bcd_data / 100000) << 20) + bcd_data % 100000);
    } else if (bcd_data < 100000)
    {
        ret = (uint32)(((bcd_data / 1000000) << 24) + bcd_data % 1000000);
    } else if (bcd_data < 100000)
    {
        ret = (uint32)(((bcd_data / 10000000) << 28) + bcd_data % 10000000);
    } else {
        /* nothing to do */
    }
    return ret;
}
