#ifndef MAINWINDOWDATAFLASH_H
#define MAINWINDOWDATAFLASH_H

#include <QMainWindow>
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QUdpSocket>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class MainWindowdataflash;
}

class MainWindowdataflash : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowdataflash(QWidget *parent = nullptr);
    ~MainWindowdataflash();

    void setearPuertoSerie(QSerialPort *ptrQSerialPort);
    void setearUDP(QUdpSocket *udpSocket, QString IP, QString Puerto);

    //Funcion que me realiza el envio de datos por el canal seleccionado
    void sendDataCommand(QByteArray dataSend);

private slots:
    void on_pushButton_Set_SSID_clicked();

    void on_pushButton_Set_Password_clicked();

    void on_pushButton_Set_IP_ESP_clicked();

    void on_pushButton_Set_IP_PC_clicked();

    void on_pushButton_Set_PORT_clicked();

    void on_pushButton_Save_Flash_clicked();

private:
    Ui::MainWindowdataflash *ui;

    QSerialPort *puertoSerie;
    QUdpSocket *UdpSocket;
    QString ip;
    QString puerto;

    void closeEvent(QCloseEvent *)override;


};

#endif // MAINWINDOWDATAFLASH_H
