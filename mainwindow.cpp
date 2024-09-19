#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
    /* sender：this，signal：rx_indication, receiver:this, member: ota_rx_mainfunction */
    connect(this,&MainWindow::rx_indication,this,&MainWindow::ota_rx_mainfunction);
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
void MainWindow::ota_rx_mainfunction(QByteArray datas)
{
    qDebug() << "[ota_rx_mainfunction]"<< Qt::endl;
    QByteArray rx_buffer = datas;
    /* 升级接收处理 */
    if (this->ota_flag == true)
    {
        qDebug() << "升级接收处理" << rx_buffer << Qt::endl;
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


