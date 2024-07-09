#ifndef DIBUJOAUTO_H
#define DIBUJOAUTO_H

#include <QWidget>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <inttypes.h>


class DibujoAuto : public QWidget
{
    Q_OBJECT
public:
    explicit DibujoAuto(QWidget *parent = nullptr);

    void setdatosADC(uint16_t datosADC[6]);

private:

    void paintEvent(QPaintEvent *)override;
    uint16_t datosADCDibujo[6];

signals:

};

#endif // DIBUJOAUTO_H
