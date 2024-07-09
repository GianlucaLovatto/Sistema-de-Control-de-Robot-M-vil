#ifndef MAINWINDOWMODULOWIFI_H
#define MAINWINDOWMODULOWIFI_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
class MainWindowModuloWIFI;
}

class MainWindowModuloWIFI : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowModuloWIFI(QWidget *parent = nullptr);
    ~MainWindowModuloWIFI();
    void setearUDP(QUdpSocket *udpSocket, QString IP, QString Puerto);
    void readData(QByteArray datos);
private slots:


    void on_pushButtonSEND_clicked();

    void on_pushButtonClear_clicked();

private:
    Ui::MainWindowModuloWIFI *ui;

    QUdpSocket *UdpSocket;



    void closeEvent(QCloseEvent *)override;

    QTimer Timer;

};

#endif // MAINWINDOWMODULOWIFI_H
