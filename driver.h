#ifndef DRIVER_H
#define DRIVER_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFile>

namespace Ui {
class Driver;
}

class Driver : public QMainWindow
{
    Q_OBJECT

public:
    explicit Driver(QWidget *parent = nullptr);
    ~Driver();

    QTimer *timer = NULL;
    QSerialPort serialPort;

private slots:
    void on_link_btn_clicked();

signals:
    void signal_link();

private:
    Ui::Driver *ui;
};

#endif // DRIVER_H
