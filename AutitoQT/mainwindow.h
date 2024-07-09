#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainwindowmcu.h"
#include "mainwindowmodulowifi.h"
#include "mainwindowdataflash.h"
#include "dibujoauto.h"
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <inttypes.h>
#include <QUdpSocket>
#include <QFile>
#include <QDateTime>
#include <QtCharts>
#include <QSplineSeries>
#include <QChartView>
#include <QGridLayout>
#include <QList>
#include <QElapsedTimer>
#include <QPalette>
#include <QColor>


typedef union
{
    uint8_t u8[4];
    uint16_t u16[2];
    uint32_t u32;

    int8_t i8[4];
    int16_t i16[2];
    int32_t i32;
}conversion_u;

typedef struct
{
    uint8_t datos[256];
    uint8_t posLectura;
    uint8_t posEscritura;
}ringBuffer_t;

typedef struct
{
    ringBuffer_t *read_Buffer;
    uint8_t estado;
    uint8_t length;
    uint8_t i;
    conversion_u conversion;
    QTimer *timerRead;
}dataController_t;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_actionMCU_triggered();

    void on_actionModulo_WIFI_triggered();

    void on_pushButtonConectar_clicked();

    void on_pushButtonActualizarPort_clicked();

    void on_pushButtonOpenPort_clicked();

    void on_pushButtonClosePort_clicked();

    void on_pushButtonIniciarCapturaDatosSensores_clicked();

    void on_pushButtonDetenerCapturaDatosSensores_clicked();

    void on_horizontalSliderTimerCapture_sliderMoved(int position);

    void on_spinBoxTimerCapture_valueChanged(int arg1);

    void on_pushButtonSetVelocidades_Timeout_clicked();

    void on_pushButtonReadFrecuencia_clicked();

    void on_pushButtonSetFrecuencia_clicked();

    void on_actionConfigConectWIFI_triggered();

    void on_pushButtonSetKp_clicked();

    void on_pushButtonGetKp_clicked();

    void on_pushButtonSetKd_clicked();

    void on_pushButtonGetKd_clicked();

    void on_pushButtonSetKi_clicked();

    void on_pushButtonGetKi_clicked();

    void on_pushButtonGetDataPID_clicked();

    void on_horizontalSliderTimerCapturePID_sliderMoved(int position);

    void on_spinBoxTimerCapturePID_valueChanged(int arg1);

    void on_pushButtonStopGetDataPID_clicked();

    void on_pushButtonSetPonderacion_clicked();

    void on_pushButtonGetPonderacion_clicked();

    void on_pushButtonClearMostrarDatos_clicked();

    void on_pushButtonPID_ON_clicked();

    void on_pushButtonPID_OFF_clicked();

    void on_pushButtonSetVelocidadMinima_clicked();

    void on_pushButtonSetCentro_clicked();

private:


    //Buffers para la lectura
    dataController_t dataControllerUDP;
    dataController_t dataControllerUSB;
    ringBuffer_t read_Buffer_UDP;
    ringBuffer_t read_BufferUSB;
    conversion_u conversion;


    //Documento para guardar los datos de los sensores
    QString fileName;
    QFile archivo;

    //Documento para guardar los datos del error del PID
    QFile archivoPID;
    QString fileNamePID;

    //Declaraciones para realizar la Grafica de los Sensores
    QChart *chartSensors;
    QSplineSeries *ADC0;
    QSplineSeries *ADC1;
    QSplineSeries *ADC2;
    QSplineSeries *ADC3;
    QSplineSeries *ADC4;
    QSplineSeries *ADC5;
    QList<QPointF> DatosADC0;
    QList<QPointF> DatosADC1;
    QList<QPointF> DatosADC2;
    QList<QPointF> DatosADC3;
    QList<QPointF> DatosADC4;
    QList<QPointF> DatosADC5;

    QChartView *chartViewSensors;
    QGridLayout *sensoresLayoutSensors;

    //Declaraciones para realizar la Grafica de error del PID
    QChart *chartErrorPID;
    QSplineSeries *error;
    QSplineSeries *errorVeloc;
    QList<QPointF> datosError;
    QList<QPointF> datosErrorVeloc;
    QChartView *chartViewErrorPID;
    QGridLayout *errorPIDLayout;


    //Declaraciones para realizar la Grafica de datos del PID
    QChart *chartPID;
    QSplineSeries *p;
    QSplineSeries *i;
    QSplineSeries *d;
    QList<QPointF> datosp;
    QList<QPointF> datosi;
    QList<QPointF> datosd;
    QChartView *chartViewPID;
    QGridLayout *PIDLayout;


    //Declaraciones para realizar la Grafica de datos del PID
    QChart *chartVelocidad;
    QSplineSeries *VelMotorD;
    QSplineSeries *VelMotorI;
    QList<QPointF> datosVelMotorD;
    QList<QPointF> datosVelMotorI;
    QChartView *chartViewVelMotores;
    QGridLayout *VelMotoresLayout;


    //Vector de datos del ADC
    uint16_t datosADC[6];
    bool datosADC_Debug;

    //Timers para la latencia de la coneccion
    QTimer *timerStatusConnectionUDP = nullptr;
    QElapsedTimer *timerLatencyUDP = nullptr;

    QTimer *timerStatusConnectionUSB = nullptr;
    QElapsedTimer *timerLatencyUSB = nullptr;


    Ui::MainWindow *ui;
    QSerialPort *SerialPort;
    QUdpSocket *UdpSocket;
    QHostAddress ip;
    quint16 port;

    MainWindowMCU *mainWindowMCU = nullptr;
    MainWindowModuloWIFI *mainWindowModuloWIFI = nullptr;
    MainWindowdataflash *mainWindowdataFlash = nullptr;


    //Funcion que me realiza la configuracion de los parametros
    void initParametros();

    //Funciones para la lectura de datos
    void readDataUDP();
    void tiempoLecturaUDP();
    void readDataUSB();
    void tiempoLecturaUSB();

    void readData(dataController_t *dataController);


    //Funcion que me crea un nuevo documento para guardar los datos de los sensores
    void createNewDocument();
    //Funcion que me guarda los datos de los sensores en el documento
    void saveDataSensorsToDocument(uint16_t *datos, uint8_t length);

    //Funcion que me inicia las graficas para dibujar los datos de los sensores
    void initChart();


    //Funciones para actualizar la grafica de los Sensores
    void drawDataChartADC0(uint16_t dato);
    void drawDataChartADC1(uint16_t dato);
    void drawDataChartADC2(uint16_t dato);
    void drawDataChartADC3(uint16_t dato);
    void drawDataChartADC4(uint16_t dato);
    void drawDataChartADC5(uint16_t dato);

    //Funciones para actualizar la grafica de error del PID
    void drawDataChartErrorPID(int16_t dato);
    void drawDataChartErrorVelocidadPID(int16_t dato);

    //Funciones para actualizar la grafica de los valores del PID
    void drawDataChartP(int16_t dato);
    void drawDataChartI(int16_t dato);
    void drawDataChartD(int16_t dato);

    //Funciones para actualizar la grafica de las velocidades del PID
    void drawDataChartVelMotorD(int16_t dato);
    void drawDataChartVelMotorI(int16_t dato);



    //Funcion que me realiza el envio de datos por el canal seleccionado
    void sendDataCommand(QByteArray dataSend);

    //Funciones que me verifican el estado de las conexion UDP y USB
    void statusConnectionUDP();
    void statusConnectionUSB();

    uint8_t calcularChecksum(uint8_t *datos, uint8_t length, uint8_t inicio);

};
#endif // MAINWINDOW_H
