#ifndef MAINWINDOWMCU_H
#define MAINWINDOWMCU_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <inttypes.h>
#include <QDebug>
#include <QTimer>


namespace Ui {
class MainWindowMCU;
}

class MainWindowMCU : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowMCU(QWidget *parent = nullptr);
    ~MainWindowMCU();
    void setearPuerto(QSerialPort *ptrQSerialPort);

    void readSerialPort(QByteArray datos);

private slots:

    void on_pushButtonSEND_clicked();

    void on_pushButtonClear_clicked();


private:

    Ui::MainWindowMCU *ui;  
    QSerialPort *puertoSerie;

    void closeEvent(QCloseEvent *)override;

};

#endif // MAINWINDOWMCU_H
