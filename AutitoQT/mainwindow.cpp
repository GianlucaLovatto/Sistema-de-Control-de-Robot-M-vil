#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Interfaz AUTITO");


    SerialPort =  new QSerialPort;
    UdpSocket = new QUdpSocket;

    //Timers para verificar el estado y latencia de la conexion
    // Para UDP
    timerStatusConnectionUDP = new QTimer(this);

    connect(timerStatusConnectionUDP, &QTimer::timeout, this, &MainWindow::statusConnectionUDP);

    timerStatusConnectionUDP->start(1000);

    timerLatencyUDP = new QElapsedTimer();

    timerLatencyUDP->invalidate();

    // Para USB
    timerStatusConnectionUSB = new QTimer(this);

    connect(timerStatusConnectionUSB, &QTimer::timeout, this, &MainWindow::statusConnectionUSB);

    timerStatusConnectionUSB->start(1000);

    timerLatencyUSB = new QElapsedTimer();

    timerLatencyUSB->invalidate();



    //Funcion que realiza la configuracion inicial
    initParametros();

}

MainWindow::~MainWindow()
{
    disconnect(SerialPort, &QSerialPort::readyRead, this, &MainWindow::readDataUSB);
    disconnect(UdpSocket, &QUdpSocket::readyRead, this, &MainWindow::readDataUDP);
    disconnect(UdpSocket, &QUdpSocket::readyRead, this, &MainWindow::readDataUDP);
    disconnect(dataControllerUDP.timerRead,&QTimer::timeout,this,&MainWindow::tiempoLecturaUDP);
    disconnect(SerialPort, &QSerialPort::readyRead, this, &MainWindow::readDataUSB);
    disconnect(dataControllerUSB.timerRead,&QTimer::timeout,this,&MainWindow::tiempoLecturaUSB);
    archivo.close();
    archivoPID.close();
    delete chartSensors;
    delete ADC0;
    delete ADC1;
    delete ADC2;
    delete ADC3;
    delete ADC4;
    delete ADC5;
    delete sensoresLayoutSensors;
    delete chartViewSensors;
    delete SerialPort;
    delete UdpSocket;
    delete ui;
}

void MainWindow::initParametros()
{
    //Se inicia el grafico sin datos a representar
    initChart();

    //Se carga la informacion para la configuracion del puerto serie.
    //velocidades del puerto
        ui->comboBoxBaudPort->addItem("2400");
        ui->comboBoxBaudPort->addItem("4800");
        ui->comboBoxBaudPort->addItem("9600");
        ui->comboBoxBaudPort->addItem("14400");
        ui->comboBoxBaudPort->addItem("19200");
        ui->comboBoxBaudPort->addItem("38400");
        ui->comboBoxBaudPort->addItem("56000");
        ui->comboBoxBaudPort->addItem("57600");
        ui->comboBoxBaudPort->addItem("115200");
    //Bits Stop
        ui->comboBoxBitsStopPort->addItem("1");
        ui->comboBoxBitsStopPort->addItem("2");
    //Paridad del puerto serie
        ui->comboBoxParityPort->addItem("NONE");
        ui->comboBoxParityPort->addItem("PAR");
        ui->comboBoxParityPort->addItem("IMPAR");

// Inicializacion de los controladores de comandos USB y UDP
        read_Buffer_UDP.posEscritura=1;
        read_Buffer_UDP.posLectura=1;
        read_BufferUSB.posEscritura=0;
        read_BufferUSB.posLectura=0;

        dataControllerUDP.read_Buffer=&read_Buffer_UDP;
        dataControllerUDP.timerRead = new QTimer(this);
        connect(dataControllerUDP.timerRead,&QTimer::timeout,this,&MainWindow::tiempoLecturaUDP);

        dataControllerUSB.read_Buffer=&read_BufferUSB;
        dataControllerUSB.timerRead = new QTimer(this);
        connect(dataControllerUSB.timerRead,&QTimer::timeout,this,&MainWindow::tiempoLecturaUSB);

        dataControllerUDP.estado=0;
        dataControllerUDP.i=0;
        dataControllerUDP.length=0;

        dataControllerUSB.estado=0;
        dataControllerUSB.i=0;
        dataControllerUSB.length=0;

        for (uint8_t i =0 ;i<6 ;i++ ) {
            datosADC[i]=0;
        }

        ui->widgetDibujoAuto->setdatosADC(datosADC);
        /*ui->pushButtonAlive->setEnabled(false);
        ui->pushButtonClosePort->setEnabled(false);
        ui->pushButtonSEND->setEnabled(true);*/

    //Se crean los documentos
    createNewDocument();

    ui->actionMCU->setEnabled(false);
    ui->actionModulo_WIFI->setEnabled(false);
}

void MainWindow::on_actionMCU_triggered()
{
    mainWindowMCU = new MainWindowMCU(this);
    mainWindowMCU->setearPuerto(SerialPort);
    mainWindowMCU->show();
}

void MainWindow::on_actionModulo_WIFI_triggered()
{
    mainWindowModuloWIFI = new MainWindowModuloWIFI(this);
    mainWindowModuloWIFI->setearUDP(UdpSocket,ui->lineEditIP->text(),ui->lineEditPuerto->text());
    mainWindowModuloWIFI->showMaximized();
}


void MainWindow::on_actionConfigConectWIFI_triggered()
{
    mainWindowdataFlash = new MainWindowdataflash(this);
    mainWindowdataFlash->setearPuertoSerie(SerialPort);
    mainWindowdataFlash->setearUDP(UdpSocket,ui->lineEditIP->text(),ui->lineEditPuerto->text());
    mainWindowdataFlash->show();
}

void MainWindow::on_pushButtonConectar_clicked()
{
    if(UdpSocket->isOpen())
    {
         UdpSocket->close();

    }else
    {
        ip.setAddress(ui->lineEditIP->text());
        port=ui->lineEditPuerto->text().toUInt();
        UdpSocket->open(QIODevice::ReadWrite);
        UdpSocket->abort();
        UdpSocket->bind(ui->lineEditPuerto->text().toUInt());
        connect(UdpSocket, &QUdpSocket::readyRead, this, &MainWindow::readDataUDP);
        ui->comboBoxChannelEnvio->addItem("UDP");
        ui->actionModulo_WIFI->setEnabled(true);
    }
}


void MainWindow::readDataUDP()
{
    while (UdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = UdpSocket->receiveDatagram();
        QByteArray datos = datagram.data();
        if(mainWindowModuloWIFI != nullptr)
        {
            mainWindowModuloWIFI->readData(datos);
        }
        for (uint8_t dato : datos)
        {
             read_Buffer_UDP.datos[read_Buffer_UDP.posEscritura]=dato;
             read_Buffer_UDP.posEscritura++;

             if( read_Buffer_UDP.posEscritura !=  read_Buffer_UDP.posLectura)
             {
                      readData(&dataControllerUDP);
                      read_Buffer_UDP.posLectura++;
             }
        }
    }
}

void MainWindow::tiempoLecturaUDP()
{
    dataControllerUDP.estado=0;
    qDebug()<< "TimeOutUDP";
    dataControllerUDP.timerRead->stop();
}

uint8_t MainWindow::calcularChecksum(uint8_t *datos, uint8_t length, uint8_t inicio)
{
    uint8_t Xor = 0;

    Xor ^= 'U';
    Xor ^= 'N';
    Xor ^= 'E';
    Xor ^= 'R';
    Xor ^= (length-1);
    Xor ^= ':';


    for(uint8_t k = inicio; length != 0 ; k++)
    {
         Xor ^= datos[k];
         length--;
    }

    return Xor;
}


void MainWindow::on_pushButtonActualizarPort_clicked()
{
    ui->comboBoxNamePort->clear();
    //Se detectan los nombres de los nuevos puertos y se agregan
    qDebug() << "Puertos disponibles: " << QSerialPortInfo::availablePorts().length();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->comboBoxNamePort->addItem(serialPortInfo.portName());
    }

}

void MainWindow::on_pushButtonOpenPort_clicked()
{
    /**************Configuracion Puerto Serie**************/
        //Se carga el nombre y la velocidad del puerto
        SerialPort->setBaudRate(ui->comboBoxBaudPort->currentText().toInt());
        SerialPort->setPortName(ui->comboBoxNamePort->currentText());
        //Se carga la paridad del puerto serie y el bit de stop
        if(ui->comboBoxParityPort->currentText() == "NONE")
        {
            SerialPort->setParity(QSerialPort::NoParity);
        }else
        {
            if(ui->comboBoxParityPort->currentText() == "PAR")
            {
                SerialPort->setParity(QSerialPort::EvenParity);
            }else
            {
                if(ui->comboBoxParityPort->currentText() == "IMPAR")
                {
                    SerialPort->setParity(QSerialPort::OddParity);
                }
            }
        }
        if(ui->comboBoxBitsStopPort->currentText() == "1")
        {
            SerialPort->setStopBits(QSerialPort::OneStop);
        }else
        {
            if(ui->comboBoxBitsStopPort->currentText()=="2")
            {
                SerialPort->setStopBits(QSerialPort::TwoStop);
            }
        }

    //Se abre el puerto serie para escritura lectura
        SerialPort->open(QIODevice::ReadWrite);

        if(SerialPort->isOpen())
        {
            QMessageBox::information(this, "Conexion realizada", "Se conecto correctamente al puerto "
                                     + SerialPort->portName() + ".", QMessageBox::Ok, QMessageBox::NoButton);
            connect(SerialPort, &QSerialPort::readyRead, this, &MainWindow::readDataUSB);
            ui->pushButtonClosePort->setEnabled(true);
            ui->comboBoxChannelEnvio->addItem("USB");
            ui->actionMCU->setEnabled(true);
        }else
        {
            QMessageBox::critical(this, "Conexion no realizada", "No se conecto correctamente al puerto "
                                  + SerialPort->portName() + ".", QMessageBox::Ok, QMessageBox::NoButton);
        }

}

void MainWindow::on_pushButtonClosePort_clicked()
{
    SerialPort->close();

    if(SerialPort->isOpen())
    {
        QMessageBox::critical(this, "Desconexión NO realizada", "NO se desconecto correctamente el puerto "
                                 + SerialPort->portName() + ".", QMessageBox::Ok, QMessageBox::NoButton);
        disconnect(SerialPort, &QSerialPort::readyRead, this, &MainWindow::readDataUSB);

    }else
    {
        QMessageBox::information(this, "Desconexión realizada", "Se desconecto correctamente al puerto "
                              + SerialPort->portName() + ".", QMessageBox::Ok, QMessageBox::NoButton);

        ui->pushButtonClosePort->setEnabled(false);

    }
}

void MainWindow::readDataUSB()
{
    QByteArray datos = SerialPort->readAll();
    if( mainWindowMCU != nullptr)
    {
        mainWindowMCU->readSerialPort(datos);
    }
    for (uint8_t dato : datos)
    {
        read_BufferUSB.datos[read_BufferUSB.posEscritura]=dato;
        read_BufferUSB.posEscritura++;
        if(read_BufferUSB.posEscritura!=read_BufferUSB.posLectura)
             {
                 readData(&dataControllerUSB);
                 read_BufferUSB.posLectura++;
             }
    }
}

void MainWindow::tiempoLecturaUSB()
{
    dataControllerUSB.estado=0;
    qDebug()<< "TimeOut";
    dataControllerUSB.timerRead->stop();
}

void MainWindow::readData(dataController_t *dataController)
{
    switch (dataController->estado)
             {
                case 0:
                    if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'U')
                    {
                        dataController->estado=1;
                        dataController->timerRead->start(100);
                    }
                    break;
                case 1:
                    if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'N')
                    {
                        dataController->estado=2;
                    }else
                    {
                       dataController->estado=0;
                    }
                    break;
                    case 2:
                        if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'E')
                        {
                            dataController->estado=3;
                        }else
                        {
                            if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'U')
                            {
                                dataController->estado=1;
                            }else
                            {
                                dataController->estado=0;
                            }
                        }
                        break;
                    case 3:
                        if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'R')
                        {
                            dataController->estado=4;
                            dataController->length=0x00;
                        }else
                        {
                            if(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == 'U')
                            {
                                dataController->estado=1;
                            }else
                            {
                                dataController->estado=0;
                            }
                        }
                        break;
                    case 4:
                        dataController->length = (dataController->read_Buffer->datos[dataController->read_Buffer->posLectura]);
                        dataController->estado=5;
                        break;
                    case 5:
                        if (dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] == ':')
                        {
                             dataController->estado = 6;
                        }else if (dataController->read_Buffer->datos[dataController->read_Buffer->posLectura] != 'U')
                        {
                            dataController->estado = 0;
                        }
                        break;
                    case 6:

                         //Por si tengo payload
                        if ((dataController->length) > 0)
                        {
                            dataController->estado = 7;
                        }
                        else
                        {
                            dataController->estado = 8;
                        }
                        dataController->i = dataController->read_Buffer->posLectura;
                        break;
                    case 7:

                        //Una vez que la posicion de lectura del buffer sea la ultima posicion del payload
                        if (dataController->read_Buffer->posLectura==(uint8_t)(dataController->i+(dataController->length)))
                        {
                            dataController->estado = 8;
                        }
                        break;
                    case 8:

                        if ((calcularChecksum((uint8_t *)&dataController->read_Buffer->datos[0], dataController->length+1 ,dataController->i))==(dataController->read_Buffer->datos[dataController->read_Buffer->posLectura]))  // La integridad de datos es correcta
                        {
                            dataController->estado=0;
                            dataController->timerRead->stop();
                            switch (dataController->read_Buffer->datos[dataController->i])
                            {
                                case 0xF0:  // Alive                                   
                                    if (timerLatencyUDP->isValid())
                                    {
                                        ui->labelLatenciaUDP->setText(QString::number(timerLatencyUDP->restart())+" ms");
                                        timerLatencyUDP->invalidate();
                                    }
                                    if (timerLatencyUSB->isValid())
                                    {
                                        ui->labelLatenciaUSB->setText(QString::number(timerLatencyUSB->restart())+" ms");

                                        timerLatencyUSB->invalidate();
                                    }
                                    break;
                                case 0xF1:  //

                                    break;
                                case 0xF2:  //

                                    break;

                                case 0xF3:

                                    break;
                               case 0xC0:
                                   if(datosADC_Debug==true)
                                   {
                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+1)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                       datosADC[0]=dataController->conversion.u16[0];
                                       drawDataChartADC0(dataController->conversion.u16[0]);

                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+4)];
                                       datosADC[1]=dataController->conversion.u16[0];
                                       drawDataChartADC1(dataController->conversion.u16[0]);

                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+5)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+6)];
                                       datosADC[2]=dataController->conversion.u16[0];
                                       drawDataChartADC2(dataController->conversion.u16[0]);

                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+7)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+8)];
                                       datosADC[3]=dataController->conversion.u16[0];
                                       drawDataChartADC3(dataController->conversion.u16[0]);

                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+9)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+10)];
                                       datosADC[4]=dataController->conversion.u16[0];
                                       drawDataChartADC4(dataController->conversion.u16[0]);

                                       dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+11)];
                                       dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+12)];
                                       datosADC[5]=dataController->conversion.u16[0];
                                       drawDataChartADC5(dataController->conversion.u16[0]);

                                       saveDataSensorsToDocument(&datosADC[0],6);
                                       ui->widgetDibujoAuto->setdatosADC(datosADC);
                                   }


                                   break;
                               case 0xC1:
                                    if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                    {
                                        qDebug ()<< "Comando C1 recibido";
                                    }

                                    break;
                               case 0xC2:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                {
                                    /*dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->spinBoxFrecuenciaMotores->setValue(dataController->conversion.u16[0]);*/

                                }
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {
                                    /*dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->spinBoxFrecuenciaMotores->setValue(dataController->conversion.u16[0]);*/

                                }

                                break;
                              case 0xA0:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {

                                }else if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                {

                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("Kp")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.u16[0])));

                                }
                                break;
                              case 0xA1:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {

                                }else if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                {
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("Kd")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.u16[0])));
                                }
                                break;
                              case 0xA2:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {

                                }else if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                {
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("Ki")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.u16[0])));
                                }
                                break;

                                case 0xA3:
                                if(dataController->read_Buffer->datos[(uint8_t)(dataController->i+1)]==0xFF)
                                {

                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    drawDataChartP(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+4)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+5)];
                                    drawDataChartD(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+6)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+7)];
                                    drawDataChartI(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }

                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+8)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+9)];
                                    drawDataChartErrorPID(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+10)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+11)];
                                    drawDataChartErrorVelocidadPID(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }

                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+12)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+13)];
                                    drawDataChartVelMotorD(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write(";");
                                    }
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+14)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+15)];
                                    drawDataChartVelMotorI(dataController->conversion.i16[0]);
                                    if (archivoPID.isOpen()==true)
                                    {
                                        archivoPID.write(QByteArray::number(dataController->conversion.i16[0]));
                                        archivoPID.write("\r\n");
                                    }
                                    dataController->conversion.u8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+16)];
                                    dataController->conversion.u8[1]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+17)];
                                    ui->lineEditcentro->clear();
                                    ui->lineEditcentro->setText(QString::number(dataController->conversion.i16[0]));
                                }else if(dataController->read_Buffer->datos[(uint8_t)(dataController->i+1)]==0x00)
                                {

                                }
                                break;
                            case 0xA4:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {

                                }else
                                {
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+1)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC0")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+2)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC1")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+3)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC2")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+4)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC3")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+5)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC4")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));
                                    dataController->conversion.i8[0]=dataController->read_Buffer->datos[(uint8_t)(dataController->i+6)];
                                    ui->tableWidgetMostrarDatos->insertRow(ui->tableWidgetMostrarDatos->rowCount());
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 0, new QTableWidgetItem(QString::asprintf("PADC5")));
                                    ui->tableWidgetMostrarDatos->setItem(ui->tableWidgetMostrarDatos->rowCount()-1, 1, new QTableWidgetItem(QString::asprintf("%d",dataController->conversion.i8[0])));

                                }
                                break;
                              case 0xAA:
                                if(dataController->read_Buffer->datos[dataController->i+1]==0xFF)
                                {

                                }else if(dataController->read_Buffer->datos[dataController->i+1]==0x00)
                                {

                                }
                                break;
                            }
                        }else
                        {
                            qDebug()<< "Paquete rechazado - Checksum invalido";
                            printf("Paquete rechazado - Checksum invalido");
                        }

                        break;

             }

}



void MainWindow::on_pushButtonIniciarCapturaDatosSensores_clicked()
{
    datosADC_Debug = true;
    QByteArray sendData;

    sendData.append((uint8_t)(0xC0));
    sendData.append((uint8_t)(0xFF));
    sendData.append((uint8_t)(ui->spinBoxTimerCapture->value()));

    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonDetenerCapturaDatosSensores_clicked()
{   
    datosADC_Debug = false;
    QByteArray sendData;
    sendData.append((uint8_t)(0xC0));
    sendData.append((uint8_t)(0x00));
    sendData.append((uint8_t)(0));

    sendDataCommand(sendData);


}
void MainWindow::createNewDocument()
{
    fileNamePID.append("C:/Users/Gianluca/Desktop/Facultad/DatosADC/");
    fileNamePID.append(QDate::currentDate().toString("(dd-MM-yyyy"));
    fileNamePID.append(QTime::currentTime().toString(" hh;mm)"));
    fileNamePID.append("DatosPID.csv");
    archivoPID.setFileName(fileNamePID);
    archivoPID.open(QIODevice::ReadWrite);
    if(archivoPID.isOpen()==true)
    {
        archivoPID.write("p;d;i;Error;ErrorVelocidad;VelMotorD;VelMotorI");
        archivoPID.write("\r\n");
    }

    fileName.append("C:/Users/Gianluca/Desktop/Facultad/DatosADC/");
    fileName.append(QDate::currentDate().toString("(dd-MM-yyyy"));
    fileName.append(QTime::currentTime().toString(" hh;mm)"));
    fileName.append("DatosSensores.csv");
    archivo.setFileName(fileName);
    archivo.open(QIODevice::ReadWrite);
    if(archivo.isOpen()==true)
    {
        archivo.write("ADC0;ADC1;ADC2;ADC3;ADC4;ADC5;TIEMPO");
        archivo.write("\r\n");
    }
}

void MainWindow::saveDataSensorsToDocument(uint16_t *datos, uint8_t length)
{
    if (archivo.isOpen()==true)
    {
        for (uint8_t i=0 ; i<length ;i++)
        {
            archivo.write(QByteArray::number(datos[i]));
            archivo.write(";");
        }
        archivo.write(QTime::currentTime().toString("hh-mm-ss").toLatin1());
        archivo.write(";");
        archivo.write("\r\n");
    }

}

void MainWindow::initChart()
{
    int j=0;
    /*******************************
     *
     * GRAFICA DE SENSORES
     *
     *******************************/

    // Inicializa la instancia de QChart
    chartSensors = new QChart();
    // Inicializa las instancias de QSplineSeries
    ADC0 = new QSplineSeries();
    ADC1 = new QSplineSeries();
    ADC2 = new QSplineSeries();
    ADC3 = new QSplineSeries();
    ADC4 = new QSplineSeries();
    ADC5 = new QSplineSeries();

    // Establecer los nombres de las curvas y colores
    ADC0->setName("ADC0");
    ADC1->setName("ADC1");
    ADC2->setName("ADC2");
    ADC3->setName("ADC3");
    ADC4->setName("ADC4");
    ADC5->setName("ADC5");
    ADC0->setColor(QColor(Qt::darkGreen));
    ADC1->setColor(QColor(Qt::darkYellow));
    ADC2->setColor(QColor(Qt::darkBlue));
    ADC3->setColor(QColor(Qt::darkRed));
    ADC4->setColor(QColor(Qt::darkMagenta));
    ADC5->setColor(QColor(Qt::darkCyan));

    for(j=0;j<30;j++)
    {
        DatosADC0.append(QPointF(j, 0));
        DatosADC1.append(QPointF(j, 0));
        DatosADC2.append(QPointF(j, 0));
        DatosADC3.append(QPointF(j, 0));
        DatosADC4.append(QPointF(j, 0));
        DatosADC5.append(QPointF(j, 0));
    }

    ADC0->append(DatosADC0);
    ADC1->append(DatosADC1);
    ADC2->append(DatosADC2);
    ADC3->append(DatosADC3);
    ADC4->append(DatosADC4);
    ADC5->append(DatosADC5);
    // Añadir curva al gráfico de instancia de QChart
    chartSensors->addSeries(ADC0);
    chartSensors->addSeries(ADC1);
    chartSensors->addSeries(ADC2);
    chartSensors->addSeries(ADC3);
    chartSensors->addSeries(ADC4);
    chartSensors->addSeries(ADC5);
    chartSensors->setTitle("ADC-TIMER");
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisX->setMin(0);
    axisX->setMax(30);
    axisY->setMin(-4000);
    axisY->setMax(4000);
    axisX->setTitleText("Tiempo(s)");
    axisY->setTitleText("DatosADC");
    axisY->setGridLineVisible(false);
    axisX->setGridLineVisible(false);

    chartSensors->addAxis(axisX,Qt::AlignBottom);
    chartSensors->addAxis(axisY,Qt::AlignLeft);
    ADC0->attachAxis(axisX);
    ADC1->attachAxis(axisX);
    ADC2->attachAxis(axisX);
    ADC3->attachAxis(axisX);
    ADC4->attachAxis(axisX);
    ADC5->attachAxis(axisX);
    ADC0->attachAxis(axisY);
    ADC1->attachAxis(axisY);
    ADC2->attachAxis(axisY);
    ADC3->attachAxis(axisY);
    ADC4->attachAxis(axisY);
    ADC5->attachAxis(axisY);

    QChartView *chartView = new QChartView(chartSensors);
    chartView->setRenderHint(QPainter::Antialiasing);

    sensoresLayoutSensors = new QGridLayout();
    sensoresLayoutSensors->addWidget(chartView,0,0);
    ui->widgetGraficasSensores->setLayout(sensoresLayoutSensors);



    /*******************************
     *
     * GRAFICA DE ERRORES PID
     *
     *******************************/
    // Inicializa la instancia de QChart
    chartErrorPID = new QChart();
    // Inicializa las instancias de QSplineSeries
    error = new QSplineSeries();
    errorVeloc = new QSplineSeries();

    // Establecer los nombres de las curvas y colores
    error->setName("ERROR");
    errorVeloc->setName("ERROR_VELOCIDAD");
    error->setColor(QColor(Qt::darkGreen));
    errorVeloc->setColor(QColor(Qt::darkBlue));

    for(j=0;j<30;j++)
    {
        datosError.append(QPointF(j, 0));
        datosErrorVeloc.append(QPointF(j, 0));
    }

    error->append(datosError);
    errorVeloc->append(datosErrorVeloc);

    // Añadir curva al gráfico de instancia de QChart
    chartErrorPID->addSeries(error);
    chartErrorPID->addSeries(errorVeloc);
    chartErrorPID->setTitle("ErrorPID-Tiempo");
    QValueAxis *axisXError = new QValueAxis();
    QValueAxis *axisYError = new QValueAxis();
    axisXError->setMin(0);
    axisXError->setMax(30);
    axisYError->setMin(-36000);
    axisYError->setMax(36000);
    axisXError->setTitleText("Tiempo(s)");
    axisYError->setTitleText("ErrorPID");
    axisYError->setGridLineVisible(false);
    axisXError->setGridLineVisible(false);

    chartErrorPID->addAxis(axisXError,Qt::AlignBottom);
    chartErrorPID->addAxis(axisYError,Qt::AlignLeft);
    error->attachAxis(axisXError);
    errorVeloc->attachAxis(axisXError);
    error->attachAxis(axisYError);
    errorVeloc->attachAxis(axisYError);

    QChartView *chartViewErrorPID = new QChartView(chartErrorPID);
    chartViewErrorPID->setRenderHint(QPainter::Antialiasing);

    errorPIDLayout = new QGridLayout();
    errorPIDLayout->addWidget(chartViewErrorPID,0,0);
    ui->widgetErroresPID->setLayout(errorPIDLayout);


    /*******************************
     *
     * GRAFICA DE VALORES PID
     *
     *******************************/
    // Inicializa la instancia de QChart
    chartPID = new QChart();
    // Inicializa las instancias de QSplineSeries
    p = new QSplineSeries();
    i = new QSplineSeries();
    d = new QSplineSeries();

    // Establecer los nombres de las curvas y colores
    p->setName("Proporcional");
    i->setName("Integral");
    d->setName("Derivativo");

    p->setColor(QColor(Qt::darkGreen));
    i->setColor(QColor(Qt::darkBlue));
    d->setColor(QColor(Qt::darkRed));

    for( j=0;j<30;j++)
    {
        datosp.append(QPointF(j, 0));
        datosi.append(QPointF(j, 0));
        datosd.append(QPointF(j, 0));
    }

    p->append(datosp);
    i->append(datosi);
    d->append(datosd);

    // Añadir curva al gráfico de instancia de QChart
    chartPID->addSeries(p);
    chartPID->addSeries(i);
    chartPID->addSeries(d);
    chartPID->setTitle("PID-Tiempo");

    QValueAxis *axisXPID = new QValueAxis();
    QValueAxis *axisYPID = new QValueAxis();
    axisXPID->setMin(0);
    axisXPID->setMax(30);
    axisYPID->setMin(-5000);
    axisYPID->setMax(5000);
    axisXPID->setTitleText("Tiempo(s)");
    axisYPID->setTitleText("PID");
    axisYPID->setGridLineVisible(false);
    axisXPID->setGridLineVisible(false);

    chartPID->addAxis(axisXPID,Qt::AlignBottom);
    chartPID->addAxis(axisYPID,Qt::AlignLeft);
    p->attachAxis(axisXPID);
    i->attachAxis(axisXPID);
    d->attachAxis(axisXPID);
    p->attachAxis(axisYPID);
    i->attachAxis(axisYPID);
    d->attachAxis(axisYPID);

    QChartView *chartViewPID = new QChartView(chartPID);
    chartViewPID->setRenderHint(QPainter::Antialiasing);

    PIDLayout = new QGridLayout();
    PIDLayout->addWidget(chartViewPID,0,0);
    ui->widgetValoresPID->setLayout(PIDLayout);


    /*******************************
     *
     * GRAFICA DE VALORES DE VELOCIDAD DE LOS MOTORES DEL PID
     *
     *******************************/
    // Inicializa la instancia de QChart
    chartVelocidad = new QChart();
    // Inicializa las instancias de QSplineSeries
    VelMotorD = new QSplineSeries();
    VelMotorI = new QSplineSeries();


    // Establecer los nombres de las curvas y colores
    VelMotorD->setName("VelocidadMotorDerecha");
    VelMotorI->setName("VelocidadMotorIzquierda");


    VelMotorD->setColor(QColor(Qt::darkGreen));
    VelMotorI->setColor(QColor(Qt::darkBlue));

    for(j=0;j<30;j++)
    {
        datosVelMotorD.append(QPointF(j, 0));
        datosVelMotorI.append(QPointF(j, 0));
    }

    VelMotorD->append(datosVelMotorD);
    VelMotorI->append(datosVelMotorI);

    // Añadir curva al gráfico de instancia de QChart
    chartVelocidad->addSeries(VelMotorD);
    chartVelocidad->addSeries(VelMotorI);
    chartVelocidad->setTitle("Velocidad-Tiempo");

    QValueAxis *axisXVelocidad = new QValueAxis();
    QValueAxis *axisYVelocidad = new QValueAxis();
    axisXVelocidad->setMin(0);
    axisXVelocidad->setMax(30);
    axisYVelocidad->setMin(-10000);
    axisYVelocidad->setMax(10000);
    axisXVelocidad->setTitleText("Tiempo(s)");
    axisYVelocidad->setTitleText("Velocidad");
    axisYVelocidad->setGridLineVisible(false);
    axisXVelocidad->setGridLineVisible(false);

    chartVelocidad->addAxis(axisXVelocidad,Qt::AlignBottom);
    chartVelocidad->addAxis(axisYVelocidad,Qt::AlignLeft);
    VelMotorD->attachAxis(axisXVelocidad);
    VelMotorI->attachAxis(axisXVelocidad);
    VelMotorD->attachAxis(axisYVelocidad);
    VelMotorI->attachAxis(axisYVelocidad);

    QChartView *chartViewVelocidad = new QChartView(chartVelocidad);
    chartViewVelocidad->setRenderHint(QPainter::Antialiasing);

    VelMotoresLayout = new QGridLayout();
    VelMotoresLayout->addWidget(chartViewVelocidad,0,0);
    ui->widgetVelocidadesMotores->setLayout(VelMotoresLayout);
}


 /*
 * Actualizacion de la grafica de los sensores
 */

void MainWindow::drawDataChartADC0(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC0.replace(i, QPointF(i, DatosADC0.value(i + 1).ry()));
    }
    DatosADC0.removeLast();
    DatosADC0.append(QPointF(30, dato * 1.0));

    ADC0->clear();
    ADC0->append(DatosADC0);
}

void MainWindow::drawDataChartADC1(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC1.replace(i, QPointF(i, DatosADC1.value(i + 1).ry()));
    }
    DatosADC1.removeLast();
    DatosADC1.append(QPointF(30, dato * 1.0));

    ADC1->clear();
    ADC1->append(DatosADC1);
}

void MainWindow::drawDataChartADC2(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC2.replace(i, QPointF(i, DatosADC2.value(i + 1).ry()));
    }
    DatosADC2.removeLast();
    DatosADC2.append(QPointF(30, dato * 1.0));

    ADC2->clear();
    ADC2->append(DatosADC2);
}

void MainWindow::drawDataChartADC3(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC3.replace(i, QPointF(i, DatosADC3.value(i + 1).ry()));
    }
    DatosADC3.removeLast();
    DatosADC3.append(QPointF(30, dato * 1.0));

    ADC3->clear();
    ADC3->append(DatosADC3);
}

void MainWindow::drawDataChartADC4(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC4.replace(i, QPointF(i, DatosADC4.value(i + 1).ry()));
    }
    DatosADC4.removeLast();
    DatosADC4.append(QPointF(30, dato * 1.0));

    ADC4->clear();
    ADC4->append(DatosADC4);
}

void MainWindow::drawDataChartADC5(uint16_t dato)
{
    for(int i=0;i<30;i++)
    {
        DatosADC5.replace(i, QPointF(i, DatosADC5.value(i + 1).ry()));
    }
    DatosADC5.removeLast();
    DatosADC5.append(QPointF(30, dato * 1.0));

    ADC5->clear();
    ADC5->append(DatosADC5);
}


/*
 * Actualizacion de la grafica de los errores del PID
 */

void MainWindow::drawDataChartErrorPID(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosError.replace(i, QPointF(i, datosError.value(i + 1).ry()));
    }
    datosError.removeLast();
    datosError.append(QPointF(30, dato * 1.0));

    error->clear();
    error->append(datosError);
}

void MainWindow::drawDataChartErrorVelocidadPID(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosErrorVeloc.replace(i, QPointF(i, datosErrorVeloc.value(i + 1).ry()));
    }
    datosErrorVeloc.removeLast();
    datosErrorVeloc.append(QPointF(30, dato * 1.0));

    errorVeloc->clear();
    errorVeloc->append(datosErrorVeloc);
}

/*
 * Actualizacion de la grafica de los valores del PID
 */

void MainWindow::drawDataChartP(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosp.replace(i, QPointF(i, datosp.value(i + 1).ry()));
    }
    datosp.removeLast();
    datosp.append(QPointF(30, dato * 1.0));

    p->clear();
    p->append(datosp);
}

void MainWindow::drawDataChartI(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosi.replace(i, QPointF(i, datosi.value(i + 1).ry()));
    }
    datosi.removeLast();
    datosi.append(QPointF(30, dato * 1.0));

    i->clear();
    i->append(datosi);
}

void MainWindow::drawDataChartD(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosd.replace(i, QPointF(i, datosd.value(i + 1).ry()));
    }
    datosd.removeLast();
    datosd.append(QPointF(30, dato * 1.0));

    d->clear();
    d->append(datosd);
}

/*
 * Actualizacion de la grafica de los valores de la velocidad de los motores del PID
 */

void MainWindow::drawDataChartVelMotorD(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosVelMotorD.replace(i, QPointF(i, datosVelMotorD.value(i + 1).ry()));
    }
    datosVelMotorD.removeLast();
    datosVelMotorD.append(QPointF(30, dato * 1.0));

    VelMotorD->clear();
    VelMotorD->append(datosVelMotorD);
}

void MainWindow::drawDataChartVelMotorI(int16_t dato)
{
    for(int i=0;i<30;i++)
    {
        datosVelMotorI.replace(i, QPointF(i, datosVelMotorI.value(i + 1).ry()));
    }
    datosVelMotorI.removeLast();
    datosVelMotorI.append(QPointF(30, dato * 1.0));

    VelMotorI->clear();
    VelMotorI->append(datosVelMotorI);
}



void MainWindow::on_horizontalSliderTimerCapture_sliderMoved(int position)
{
    ui->spinBoxTimerCapture->setValue(position);
}

void MainWindow::on_spinBoxTimerCapture_valueChanged(int arg1)
{
    ui->horizontalSliderTimerCapture->setValue(arg1);
}

void MainWindow::on_pushButtonSetVelocidades_Timeout_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xC1));
    conversion.i16[0]=ui->spinBoxVelocidadM1->value();
    sendData.append((uint8_t)(conversion.i8[0]));
    sendData.append((uint8_t)(conversion.i8[1]));
    conversion.i16[0]=ui->spinBoxVelocidadM2->value();
    sendData.append((uint8_t)(conversion.i8[0]));
    sendData.append((uint8_t)(conversion.i8[1]));
    conversion.u16[0]=ui->spinBoxMotorTimeout->value();
    sendData.append((uint8_t)(conversion.u8[0]));
    sendData.append((uint8_t)(conversion.u8[1]));

   sendDataCommand(sendData);
}

void MainWindow::on_pushButtonReadFrecuencia_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xC2));
    sendData.append((uint8_t)(0x00));

    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonSetFrecuencia_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xC2));
    sendData.append((uint8_t)(0xFF));
    conversion.u16[0]=(ui->spinBoxFrecuenciaMotores->value());
    sendData.append((uint8_t)(conversion.u8[0]));
    sendData.append((uint8_t)(conversion.u8[1]));

    sendDataCommand(sendData);
}

//Funcion que me envia el protocolo con el comando y el payload
void MainWindow::sendDataCommand(QByteArray dataSend)
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


    if(ui->comboBoxChannelEnvio->currentText()=="UDP")
    {
        if(UdpSocket->isOpen())
        {
            qDebug()<< ip;
            qDebug()<< port;
            UdpSocket->writeDatagram(datos,ip,port);
        }else
        {

                QMessageBox::information(this, "NOT CONNECT UDP", "(CHECK CONECTION UDP)"
                                      ".", QMessageBox::Ok, QMessageBox::NoButton);
        }

    }else if(ui->comboBoxChannelEnvio->currentText()=="USB")
    {
        if(SerialPort->isOpen())
        {
           SerialPort->write(datos,datos.size());
        }else
        {
            QMessageBox::information(this, "NOT CONNECT USB", "(CHECK CONECTION USB)"
                                  ".", QMessageBox::Ok, QMessageBox::NoButton);
        }

    }
}

//Funcion que me verifican el estado de las conexion UDP
void MainWindow::statusConnectionUDP()
{
    if (UdpSocket->isOpen())
    {
        ui->labelEstadoUDP->setText("Conectado");

        QPalette palette = ui->labelEstadoUDP->palette();

        palette.setColor(ui->labelEstadoUDP->foregroundRole(), QColor(0xA5, 0xC8, 0x00, 0xFF));

        ui->labelEstadoUDP->setPalette(palette);


        if (!timerLatencyUDP->isValid())
        {
            timerLatencyUDP->start();
        }

        if (timerLatencyUDP->elapsed() < 1000)
        {
            QPalette palette = ui->labelLatenciaUDP->palette();

            palette.setColor(ui->labelLatenciaUDP->foregroundRole(), QColor(0x5C, 0xA8, 0x59, 0xFF));

            ui->labelLatenciaUDP->setPalette(palette);
        }

        if (timerLatencyUDP->elapsed() >= 1000)
        {
            ui->labelLatenciaUDP->setText("<" + QString::number(timerLatencyUDP->elapsed()) + " ms");

            QPalette palette = ui->labelLatenciaUDP->palette();

            palette.setColor(ui->labelLatenciaUDP->foregroundRole(), QColor(0xA5, 0xC8, 0x00, 0xFF));

            ui->labelLatenciaUDP->setPalette(palette);
        }

        if (timerLatencyUDP->elapsed() >= 4000)
        {
            ui->labelLatenciaUDP->setText("<" + QString::number(timerLatencyUDP->elapsed()) + " ms");

            QPalette palette = ui->labelLatenciaUDP->palette();

            palette.setColor(ui->labelLatenciaUDP->foregroundRole(), QColor(0xFF, 0x00, 0x00, 0xFF));

            ui->labelLatenciaUDP->setPalette(palette);
        }

        if (timerLatencyUDP->elapsed() >= 20000)
        {
            UdpSocket->close();
            QMessageBox::information(this, "SE CERRO EL SOCKET UDP AL NO OBTENER UNA RESPUESTA. \r\n", "IP:" + ui->lineEditIP->text() + "PUERTO:" +
                                     ui->lineEditPuerto->text(), QMessageBox::Ok, QMessageBox::NoButton);

        }

        QByteArray sendData;

        sendData.append(0xF0);

        sendDataCommand(sendData);
    }

    else
    {
        ui->labelEstadoUDP->setText("Desconectado");

        QPalette palette = ui->labelEstadoUDP->palette();

        palette.setColor(ui->labelEstadoUDP->foregroundRole(), QColor(0xFF, 0x00, 0x00, 0xFF));

        ui->labelEstadoUDP->setPalette(palette);

        if (timerLatencyUDP->isValid())
        {
            timerLatencyUDP->invalidate();
        }

        ui->labelLatenciaUDP->setText("");
    }
}

//Funcion que me verifican el estado de las conexion USB
void MainWindow::statusConnectionUSB()
{
    if (SerialPort->isOpen())
    {
        ui->labelEstadoUSB->setText("Conectado");
        QPalette palette = ui->labelEstadoUSB->palette();

        palette.setColor(ui->labelEstadoUSB->foregroundRole(), QColor(0xA5, 0xC8, 0x00, 0xFF));

        ui->labelEstadoUSB->setPalette(palette);

        if (!timerLatencyUSB->isValid())
        {
            timerLatencyUSB->start();
        }

        if (timerLatencyUSB->elapsed() < 1000)
        {
            QPalette palette = ui->labelLatenciaUSB->palette();

            palette.setColor(ui->labelLatenciaUSB->foregroundRole(), QColor(0x5C, 0xA8, 0x59, 0xFF));

            ui->labelLatenciaUSB->setPalette(palette);
        }

        if (timerLatencyUSB->elapsed() >= 1000)
        {
            ui->labelLatenciaUSB->setText("<" + QString::number(timerLatencyUSB->elapsed()) + " ms");

            QPalette palette = ui->labelLatenciaUSB->palette();

            palette.setColor(ui->labelLatenciaUSB->foregroundRole(), QColor(0xA5, 0xC8, 0x00, 0xFF));

            ui->labelLatenciaUSB->setPalette(palette);
        }

        if (timerLatencyUSB->elapsed() >= 4000)
        {
            ui->labelLatenciaUSB->setText("<" + QString::number(timerLatencyUSB->elapsed()) + " ms");

            QPalette palette = ui->labelLatenciaUSB->palette();

            palette.setColor(ui->labelLatenciaUSB->foregroundRole(), QColor(0xFF, 0x00, 0x00, 0xFF));

            ui->labelLatenciaUSB->setPalette(palette);
        }

        if (timerLatencyUSB->elapsed() >= 20000)
        {
            SerialPort->close();
            QMessageBox::information(this, "SE CERRO LA CONEXION USB AL NO OBTENER UNA RESPUESTA. \r\n", "Nombre:" + SerialPort->portName()
                                     + "\r\n\tBaud rate: " + SerialPort->baudRate(), QMessageBox::Ok, QMessageBox::NoButton);

        }

        QByteArray sendData;

        sendData.append(0xF0);

        sendDataCommand(sendData);
    }

    else
    {
        ui->labelEstadoUSB->setText("Desconectado");
        QPalette palette = ui->labelEstadoUSB->palette();

        palette.setColor(ui->labelEstadoUSB->foregroundRole(), QColor(0xFF, 0x00, 0x00, 0xFF));

        ui->labelEstadoUSB->setPalette(palette);
        if (timerLatencyUSB->isValid())
        {
            timerLatencyUSB->invalidate();
        }

        ui->labelLatenciaUSB->setText("");
    }
}


void MainWindow::on_pushButtonSetKp_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA0));
    sendData.append((uint8_t)(0xFF));
    conversion.u16[0]=(ui->spinBoxConstanteKp->value());
    sendData.append((uint8_t)(conversion.u8[0]));
    sendData.append((uint8_t)(conversion.u8[1]));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonGetKp_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA0));
    sendData.append((uint8_t)(0x00));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonSetKd_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA1));
    sendData.append((uint8_t)(0xFF));
    conversion.u16[0]=(ui->spinBoxConstanteKd->value());
    sendData.append((uint8_t)(conversion.u8[0]));
    sendData.append((uint8_t)(conversion.u8[1]));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonGetKd_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA1));
    sendData.append((uint8_t)(0x00));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonSetKi_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA2));
    sendData.append((uint8_t)(0xFF));
    conversion.u16[0]=(ui->spinBoxConstanteKi->value());
    sendData.append((uint8_t)(conversion.u8[0]));
    sendData.append((uint8_t)(conversion.u8[1]));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonGetKi_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA2));
    sendData.append((uint8_t)(0x00));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonGetDataPID_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA3));
    sendData.append((uint8_t)(0xFF));
    sendData.append((uint8_t)(ui->spinBoxTimerCapturePID->value()));
    sendDataCommand(sendData);
}

void MainWindow::on_horizontalSliderTimerCapturePID_sliderMoved(int position)
{
     ui->spinBoxTimerCapturePID->setValue(position);
}


void MainWindow::on_spinBoxTimerCapturePID_valueChanged(int arg1)
{
    ui->horizontalSliderTimerCapturePID->setValue(arg1);
}


void MainWindow::on_pushButtonStopGetDataPID_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA3));
    sendData.append((uint8_t)(0x00));
    sendData.append((uint8_t)(ui->spinBoxTimerCapturePID->value()));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonSetPonderacion_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA4));
    sendData.append((uint8_t)(0xFF));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC0->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC1->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC2->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC3->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC4->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC5->value()));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonGetPonderacion_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA4));
    sendData.append((uint8_t)(0x00));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC0->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC1->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC2->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC3->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC4->value()));
    sendData.append((int8_t)(ui->spinBoxPonderacionADC5->value()));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonClearMostrarDatos_clicked()
{
    while (ui->tableWidgetMostrarDatos->rowCount()>0)
    {
         ui->tableWidgetMostrarDatos->removeRow( ui->tableWidgetMostrarDatos->rowCount()-1);
    }
}

void MainWindow::on_pushButtonPID_ON_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xAA));
    sendData.append((uint8_t)(0xFF));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonPID_OFF_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xAA));
    sendData.append((uint8_t)(0x00));
    sendDataCommand(sendData);
}

void MainWindow::on_pushButtonSetCentro_clicked()
{
    QByteArray sendData;
    sendData.append((uint8_t)(0xA5));
    sendData.append((uint8_t)(0xFF));
    sendDataCommand(sendData);
}
