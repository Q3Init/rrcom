#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Platform_Types.h"

static const uint8 HeaderPattern[HEADER_CNT] = {0XEE};
static Ota_stepType ota_step = OTA_EXTEND_SESSION;

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
    /* sender：串口，signal：串口readyRead, receiver:this, member: rxhandler */
    connect(&this->driver->serialPort,&QSerialPort::readyRead,this,&MainWindow::rx_handler);
    /* sender：this，signal：rx_indication, receiver:this, member: serial_data_display_window */
    connect(this,&MainWindow::rx_indication,this,&MainWindow::serial_data_display_window);
    /* sender：this，signal：rx_indication, receiver:this, member: InterRxindication */
    connect(this,&MainWindow::rx_indication,this,&MainWindow::InterRxindication);
    /* sender：this，signal：inter_rx_signal, receiver:this, member: ota_mainfunction */
    connect(this,&MainWindow::inter_rx_signal,this,&MainWindow::ota_mainfunction);
    /* sender：this，signal：inter_tx_signal, receiver:this, member: Inter_transmit */
    connect(this,&MainWindow::inter_tx_signal,this,&MainWindow::Inter_transmit);
    /* sender：this，signal：ota_extend_session_signal, receiver:this, member: ota_extend_session */
    connect(this,&MainWindow::ota_extend_session_signal,this,&MainWindow::ota_extend_session);
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
    emit this->rx_indication(recv_datas);
}

/* ota主要接收数据处理函数 */
void MainWindow::InterRxindication(QByteArray datas)
{
    qDebug() << "[InterRxindication]"<< Qt::endl;
    rxObjType *objPtr;
    uint8_t rx_buffer[64] = {0};
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
                emit this->ota_extend_session_signal();
                ota_step = OTA_STOP_COMMUNCTION;
            }
            /* 收到扩展会话应答，触发停止通信服务信号 */
        }
        break;
    case OTA_STOP_COMMUNCTION:
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
    uint8 data = 3;
    emit this->inter_tx_signal(ID_STOP_COMMUNCTION ,len ,&data);
}
