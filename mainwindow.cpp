#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->driver = new Driver;
    this->driver->show();
    /* 收到driver界面的连接成功信号 */
    connect(this->driver,&Driver::signal_link,this,[this]{
        this->driver->hide();
        this->show();
    });
    connect(&this->driver->serialPort,&QSerialPort::readyRead,this,&MainWindow::onReadyRead);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/* 退出主菜单 */
void MainWindow::on_quit_btn_clicked()
{
    this->hide();
    this->driver->show();
    /* 退出主菜单并关闭驱动 */
    this->driver->serialPort.close();
}

/* 数据接收窗口 */
void MainWindow::onReadyRead()
{
#if 1
    QByteArray recBuf = this->driver->serialPort.readAll();
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
#else
    QString buffer_1;
    auto recv_buffer = this->driver->serialPort.readAll();
    if(!recv_buffer.isEmpty())//如果非空说明有数据接收
    {   //转换成16进制大写
        QString str=recv_buffer.toHex().data();
        str=str.toUpper();
              //一个16进制占4位，8位为一字节，所以每两位16进制空一格
        for(int i=0;i<str.length();i+=2)
        {
            QString str_1 = str.mid (i,2);
            buffer_1 += str_1;
            buffer_1 += " ";
        }
        //读取之前显示数据
        QString new_data = ui->recv_edit->toPlainText();
        //清空显示
        ui->recv_edit->clear();
        //判断16进制显示按钮状态进行显示
        if (this->hexdisplay_flag)
        {
            /* 16进制显示 */
            new_data += QString(buffer_1) + "\r\n";
            ui->recv_edit->append(new_data);
        }
        else
        {
            /* 原始直接显示 */
            new_data += QString(recv_buffer) + "\r\n";
            ui->recv_edit->append(new_data);
        }
    }
    recv_buffer.clear();
#endif
}

/* 发送按钮 */
void MainWindow::on_send_btn_released()
{
#if 1
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
    ui->recv_edit->insertPlainText(array);
    ui->recv_edit->moveCursor(QTextCursor::End);
    qDebug() << array << Qt::endl;
    //发送成功
    if (this->driver->serialPort.write(array) > 0) {
        qDebug() << "send ok" << Qt::endl;
    } else {
        qDebug() << "send fail" << Qt::endl;
    }
#else
    auto dataStr = ui->send_edit->toPlainText();
    if (this->driver->serialPort.write(QByteArray::fromHex(dataStr.toLatin1().data())) > 0)
    {
        qDebug() << "send ok" << Qt::endl;
    } else {
        qDebug() << "send fail" << Qt::endl;
    }
#endif
}

void MainWindow::on_hexdisplay_checkBox_toggled(bool checked)
{
    if(checked)
    {
        // displayHex();
        this->hexdisplay_flag = true;
    }
    else
    {
        // displayText();
        this->hexdisplay_flag = NULL;
    }
}

void MainWindow::displayHex()
{
    //先把数据拿出来
    auto dataStr = ui->recv_edit->toPlainText();
    //转成16进制
    auto hexData = dataStr.toLocal8Bit().toHex().toUpper();
    //写回去
    ui->recv_edit->setPlainText(hexData);
}

void MainWindow::displayText()
{
    //先把数据拿出来
    auto dataStr = ui->recv_edit->toPlainText();
    //转成文本
    auto textData = QString::fromLocal8Bit(dataStr.toLocal8Bit());
    //写回去
    ui->recv_edit->setPlainText(textData);
}

