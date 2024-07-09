#include "mainwindowmodulowifi.h"
#include "ui_mainwindowmodulowifi.h"

MainWindowModuloWIFI::MainWindowModuloWIFI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowModuloWIFI)
{
    ui->setupUi(this);
    setWindowTitle("Depuracion Modulo WIFI");

    //Se cargan los comandos
    ui->comboBoxComand->addItem("F0 Alive");
    ui->comboBoxComand->addItem("F1 ");
    ui->comboBoxComand->addItem("F2 ");

}

MainWindowModuloWIFI::~MainWindowModuloWIFI()
{
    delete ui;
}

void MainWindowModuloWIFI::setearUDP(QUdpSocket *udpSocket, QString IP, QString Puerto)
{
    ui->lineEditIP->setText(IP);
    ui->lineEditPuerto->setText(Puerto);
    UdpSocket = udpSocket;
}

void MainWindowModuloWIFI::readData(QByteArray datos)
{

    for (uint8_t i : datos)
    {
         ui->tableWidgetReceivedSentData->insertRow(ui->tableWidgetReceivedSentData->rowCount());
         if(i < 0x10)
         {
             ui->tableWidgetReceivedSentData->setItem(ui->tableWidgetReceivedSentData->rowCount()-1, 2,
                                                      new QTableWidgetItem(QString::asprintf("0x0%X",i)));
         }else
         {
             ui->tableWidgetReceivedSentData->setItem(ui->tableWidgetReceivedSentData->rowCount()-1, 2,
                                                      new QTableWidgetItem(QString::asprintf("0x%X",i)));
         }
         ui->tableWidgetReceivedSentData->setItem(ui->tableWidgetReceivedSentData->rowCount() -1,0,
                                                  new QTableWidgetItem(QString::asprintf("%c",i)));
         ui->tableWidgetReceivedSentData->setItem(ui->tableWidgetReceivedSentData->rowCount() -1,1,
                                                  new QTableWidgetItem(QString::asprintf("%d",i)));

    }

}

void MainWindowModuloWIFI::on_pushButtonSEND_clicked()
{
    QByteArray datos;
    QString payload = ui->lineEditPayload->text();
    payload.append(" ");
    QStringList aux;
    uint8_t j=payload.count(" ");

    while(j>0)
    {
        if(payload.at(0) != " ")
        {
          aux.append(payload.section(' ',0,0));
          payload.remove(0,payload.section(' ',0,0).length());
        }else
        {
            payload.remove(0,1);
            j--;
        }

    }

    // Se empaqueta la cabecera
    datos.append('U');
    datos.append('N');
    datos.append('E');
    datos.append('R');
    datos.append(aux.size());
    datos.append(':');
    datos.append(ui->comboBoxComand->currentText().section(' ',0,0).toUInt(nullptr,16));



    for (j=0;j<aux.size();j++)
    {
        if(ui->checkBoxHex->isChecked()==true)
        {
            datos.append(aux.at(j).toUInt(nullptr,16));
        }else
        {
            datos.append(QString::asprintf("%X",aux.at(j).toUInt(nullptr,16)));
        }

    }


    uint8_t XOR = 0;

    for(j = 0 ; j<datos.length(); j++)
    {
         XOR ^= datos[j];
    }

    datos.append(XOR);

    datos.append('\r');
    datos.append('\n');

    UdpSocket->writeDatagram(datos,QHostAddress(ui->lineEditIP->text()),ui->lineEditPuerto->text().toUInt());


}

void MainWindowModuloWIFI::on_pushButtonClear_clicked()
{
    while (ui->tableWidgetReceivedSentData->rowCount()>0)
    {
         ui->tableWidgetReceivedSentData->removeRow( ui->tableWidgetReceivedSentData->rowCount()-1);
    }
}

void MainWindowModuloWIFI::closeEvent(QCloseEvent *)
{
    deleteLater();
}

