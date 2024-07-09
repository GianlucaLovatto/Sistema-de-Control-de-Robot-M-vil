QT       += core gui
QT       += core gui serialport network
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dibujoauto.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindowdataflash.cpp \
    mainwindowmcu.cpp \
    mainwindowmodulowifi.cpp

HEADERS += \
    dibujoauto.h \
    mainwindow.h \
    mainwindowdataflash.h \
    mainwindowmcu.h \
    mainwindowmodulowifi.h

FORMS += \
    mainwindow.ui \
    mainwindowdataflash.ui \
    mainwindowmcu.ui \
    mainwindowmodulowifi.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
