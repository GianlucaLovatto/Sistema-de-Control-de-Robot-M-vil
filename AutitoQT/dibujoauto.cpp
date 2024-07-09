#include "dibujoauto.h"

DibujoAuto::DibujoAuto(QWidget *parent) : QWidget(parent)
{

}

void DibujoAuto::paintEvent(QPaintEvent *)
{
    QPainter dibujo(this);
    QPixmap pixmap;
    pixmap.load("C:/Users/Gianluca/Desktop/Prog.Microcontroladores/AUTITO.png");
    dibujo.drawPixmap(QRect(0,0,width(),height()),pixmap);

    dibujo.drawText(QPoint((width()/2.45),(height()/2.65)),QString::number(datosADCDibujo[2]));
    dibujo.drawText(QPoint((width()/1.88),(height()/2.65)),QString::number(datosADCDibujo[3]));
    dibujo.drawText(QPoint((width()/3.3),(height()/2)),QString::number(datosADCDibujo[1]));
    dibujo.drawText(QPoint((width()/1.52),(height()/1.98)),QString::number(datosADCDibujo[4]));
    dibujo.drawText(QPoint((width()/3.3),(height()/1.48)),QString::number(datosADCDibujo[0]));
    dibujo.drawText(QPoint((width()/1.52),(height()/1.5)),QString::number(datosADCDibujo[5]));


    dibujo.drawText(QPoint((width()/2.5),(height()/4)),"A2");
    dibujo.drawText(QPoint((width()/1.75),(height()/4)),"A3");
    dibujo.drawText(QPoint((width()/3.25),(height()/2.58)),"A1");
    dibujo.drawText(QPoint((width()/1.5),(height()/2.55)),"A4");
    dibujo.drawText(QPoint((width()/3.3),(height()/1.7)),"A0");
    dibujo.drawText(QPoint((width()/1.48),(height()/1.72)),"A5");

}

void DibujoAuto::setdatosADC(uint16_t datosADC[6])
{
    datosADCDibujo[0]=datosADC[0];
    datosADCDibujo[1]=datosADC[1];
    datosADCDibujo[2]=datosADC[2];
    datosADCDibujo[3]=datosADC[3];
    datosADCDibujo[4]=datosADC[4];
    datosADCDibujo[5]=datosADC[5];
    update();
}
