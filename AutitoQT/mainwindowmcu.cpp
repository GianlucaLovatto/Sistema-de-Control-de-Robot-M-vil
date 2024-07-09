#include "mainwindowmcu.h"
#include "ui_mainwindowmcu.h"
#include <inttypes.h>

MainWindowMCU::MainWindowMCU(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowMCU)
{
    ui->setupUi(this);

    setWindowTitle("Depuracion con MCU");

    //Se cargan los comandos
    ui->comboBoxComand->addItem("F0 Alive");
    ui->comboBoxComand->addItem("F1 Debug");
    ui->comboBoxComand->addItem("F2 ComandAT");
}

MainWindowMCU::~MainWindowMCU()
{
    delete ui;
}


void MainWindowMCU::on_pushButtonSEND_clicked()
{

    QByteArray datos;
    QStringList payload;

    // Se empaqueta la cabecera
    datos.append((uint8_t)('U'));
    datos.append((uint8_t)('N'));
    datos.append((uint8_t)('E'));
    datos.append((uint8_t)('R'));

    if(ui->checkBoxHex->isChecked()==true)
    {
        QString aux = ui->lineEditPayload->text();
        aux.append(" ");
        uint8_t j=aux.count(" ");

        while(j>0)
        {
            if(aux.at(0) != " ")
            {
              payload.append(aux.section(' ',0,0));
              aux.remove(0,aux.section(' ',0,0).length());
            }else
            {
                aux.remove(0,1);
                j--;
            }
        }
        datos.append((uint8_t)payload.size());
    }else
    {
        if(ui->checkBoxComandoAT->isChecked()==true)
        {
            datos.append((uint8_t)(ui->lineEditPayload->text().length())+2);
        }else
        {
           datos.append((uint8_t)(ui->lineEditPayload->text().length()));
        }
    }

    datos.append((uint8_t)(':'));
    datos.append((uint8_t)(ui->comboBoxComand->currentText().section(' ',0,0).toUInt(nullptr,16)));



    if(ui->checkBoxHex->isChecked()==true)
    {
        for (QString data : payload) {
            datos.append((uint8_t)(data.toUInt(nullptr, 16)));
        }
    }
    else
    {
        for (QChar data : ui->lineEditPayload->text())
        {
            datos.append((uint8_t)(data.toLatin1()));
        }
        if(ui->checkBoxComandoAT->isChecked()==true)
        {
            datos.append('\r');
            datos.append('\n');

        }
    }



    uint8_t XOR = 0;

    for(char data : datos)
    {
         XOR ^= (uint8_t)(data);
    }

    datos.append(XOR);

    puertoSerie->write(datos,datos.size());

}

void MainWindowMCU::readSerialPort(QByteArray datos)
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


void MainWindowMCU::setearPuerto(QSerialPort *ptrQSerialPort)
{
    puertoSerie = ptrQSerialPort;
}

void MainWindowMCU::on_pushButtonClear_clicked()
{
    while (ui->tableWidgetReceivedSentData->rowCount()>0)
    {
         ui->tableWidgetReceivedSentData->removeRow( ui->tableWidgetReceivedSentData->rowCount()-1);
    }

}

void MainWindowMCU::closeEvent(QCloseEvent *)
{
    deleteLater();
}
