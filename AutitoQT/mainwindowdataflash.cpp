#include "mainwindowdataflash.h"
#include "ui_mainwindowdataflash.h"

MainWindowdataflash::MainWindowdataflash(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowdataflash)
{
    ui->setupUi(this);
}

MainWindowdataflash::~MainWindowdataflash()
{
    delete ui;
}

void MainWindowdataflash::on_pushButton_Set_SSID_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD0));
    dataSend.append((uint8_t)(ui->lineEdit_SSID->text().length()));
    dataSend.append(ui->lineEdit_SSID->text());
    sendDataCommand(dataSend);
}

void MainWindowdataflash::on_pushButton_Set_Password_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD1));
    dataSend.append((uint8_t)(ui->lineEdit_Password->text().length()));
    dataSend.append(ui->lineEdit_Password->text());
    sendDataCommand(dataSend);
}

void MainWindowdataflash::on_pushButton_Set_IP_ESP_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD2));
    dataSend.append((uint8_t)(ui->lineEdit_IP_Modulo_ESP->text().length()));
    dataSend.append(ui->lineEdit_IP_Modulo_ESP->text());
    sendDataCommand(dataSend);
}

void MainWindowdataflash::on_pushButton_Set_IP_PC_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD3));
    dataSend.append((uint8_t)(ui->lineEdit_IP_PC->text().length()));
    dataSend.append(ui->lineEdit_IP_PC->text());
    sendDataCommand(dataSend);
}

void MainWindowdataflash::on_pushButton_Set_PORT_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD4));
    dataSend.append((uint8_t)(ui->lineEdit_PORT->text().length()));
    dataSend.append(ui->lineEdit_PORT->text());
    sendDataCommand(dataSend);
}

void MainWindowdataflash::on_pushButton_Save_Flash_clicked()
{
    QByteArray dataSend;
    dataSend.append((uint8_t)(0xD5));
    dataSend.append((uint8_t)(0xFF));
    sendDataCommand(dataSend);
}


void MainWindowdataflash::setearPuertoSerie(QSerialPort *ptrQSerialPort)
{
    puertoSerie = ptrQSerialPort;

    if(puertoSerie->isOpen())
    {
        ui->comboBox_Channel_Send->addItem("USB");
    }

}

void MainWindowdataflash::setearUDP(QUdpSocket *udpSocket, QString IP, QString Puerto)
{
    ip = IP;
    puerto = Puerto;
    UdpSocket = udpSocket;

    if(UdpSocket->isOpen())
    {
        ui->comboBox_Channel_Send->addItem("UDP");
    }
}

//Funcion que me envia el protocolo con el comando y el payload
void MainWindowdataflash::sendDataCommand(QByteArray dataSend)
{
    QByteArray datos;

    // Se empaqueta la cabecera
    datos.append((uint8_t)('U'));
    datos.append((uint8_t)('N'));
    datos.append((uint8_t)('E'));
    datos.append((uint8_t)('R'));
    datos.append(dataSend.length() - 1);
    datos.append((uint8_t)(':'));

    datos.append(dataSend);

    uint8_t XOR = 0;

    for(char data : datos)
    {
         XOR ^= (uint8_t)(data);
    }
    datos.append(XOR);


    if(ui->comboBox_Channel_Send->currentText()=="UDP")
    {
        if(UdpSocket->isOpen())
        {
            UdpSocket->writeDatagram(datos,QHostAddress(ip),puerto.toUInt());
        }else
        {

                QMessageBox::information(this, "NOT CONNECT UDP", "(CHECK CONECTION UDP)"
                                      ".", QMessageBox::Ok, QMessageBox::NoButton);
        }

    }else if(ui->comboBox_Channel_Send->currentText()=="USB")
    {
        if(puertoSerie->isOpen())
        {
           puertoSerie->write(datos,datos.size());
        }else
        {
            QMessageBox::information(this, "NOT CONNECT USB", "(CHECK CONECTION USB)"
                                  ".", QMessageBox::Ok, QMessageBox::NoButton);
        }

    }else
    {
        QMessageBox::information(this, "NO POSEE NINGUNA CONEXION", "(CHECK CONECTIONS)"
                              ".", QMessageBox::Ok, QMessageBox::NoButton);
    }
}


void MainWindowdataflash::closeEvent(QCloseEvent *)
{
    deleteLater();
}

