#include "driver.h"
#include "ui_driver.h"

Driver::Driver(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Driver)
{
    ui->setupUi(this);
    /* 实例化定时器对象 */
    timer = new QTimer;
    /* 初始化把lable显示一个当前时间 */
    QString tm_init = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->datetime_lab->setText(tm_init);
    /* 设置1s超时发送信号 */
    timer->start(1000);
    /* 收到信号更新当前时间 */
    connect(timer,&QTimer::timeout,this,[this](){
        QString tm = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->datetime_lab->setText(tm);
        /* 实时检查串口有没断链 */
        if(!serialPort.isOpen()) {
            qDebug() << "serial port open failed！！！" <<Qt::endl;
        }
    });
    /* 获取所有的可用的串口 */
    auto portsInfo = QSerialPortInfo::availablePorts();
    for(auto & info : portsInfo)
    {
        qInfo()<<info.description() << info.portName() << info.systemLocation();
        ui->ports_cmb->addItem(info.portName() + ":" + info.description(),info.portName());
    }
    /* 获取标准的波特率 */
    auto baudRates = QSerialPortInfo::standardBaudRates();
    for (auto br : baudRates)
    {
        ui->baudrate_cmb->addItem(QString::number(br),br);
    }
    ui->baudrate_cmb->setCurrentText("115200");
    //设置停止位
    ui->stopbit_cmb->addItem("1",QSerialPort::OneStop);
    ui->stopbit_cmb->addItem("1.5",QSerialPort::OneAndHalfStop);
    ui->stopbit_cmb->addItem("2",QSerialPort::TwoStop);
    //设置数据位
    ui->databit_cmb->addItem("5",QSerialPort::Data5);
    ui->databit_cmb->addItem("6",QSerialPort::Data6);
    ui->databit_cmb->addItem("7",QSerialPort::Data7);
    ui->databit_cmb->addItem("8",QSerialPort::Data8);
    ui->databit_cmb->setCurrentText("8");
    //设置校验位
    ui->parity_cmb->addItem("NoParity",QSerialPort::NoParity);
    ui->parity_cmb->addItem("EvenParity",QSerialPort::EvenParity);
    ui->parity_cmb->addItem("OddParity",QSerialPort::OddParity);
    ui->parity_cmb->addItem("SpaceParity",QSerialPort::SpaceParity);
    ui->parity_cmb->addItem("MarkParity",QSerialPort::MarkParity);

}   

Driver::~Driver()
{
    delete ui;
}

/* 连接驱动，并进入主菜单 */
void Driver::on_link_btn_clicked()
{
    auto portName = ui->ports_cmb->currentData().toString();
    //获取波特率
    auto baudRate = ui->baudrate_cmb->currentData().value<QSerialPort::BaudRate>();
    //获取数据位
    auto dataBits = ui->databit_cmb->currentData().value<QSerialPort::DataBits>();
    //获取停止位
    auto stopBits = ui->stopbit_cmb->currentData().value<QSerialPort::StopBits>();
    //获取校验位
    auto parity = ui->parity_cmb->currentData().value<QSerialPort::Parity>();

    serialPort.setPortName(portName);
    serialPort.setBaudRate(baudRate);
    serialPort.setDataBits(dataBits);
    serialPort.setStopBits(stopBits);
    serialPort.setParity(parity);

    if (serialPort.open(QIODevice::ReadWrite) == true) {
        emit this->signal_link();
    } else {
        QMessageBox::warning(this,"warning",portName + " open failed:" + serialPort.errorString());
    }

}

