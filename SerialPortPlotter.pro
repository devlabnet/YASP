#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T14:53:33
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SerialPortPlotter
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    	qcustomplot.cpp \
    helpwindow.cpp \
    dialogwidgets.cpp \
    sliderwidget.cpp

HEADERS  += mainwindow.hpp \
    		qcustomplot.h \
    helpwindow.hpp \
    dialogwidgets.h \
    sliderwidget.h


FORMS    += mainwindow.ui \
    helpwindow.ui \
    dialogwidgets.ui

RC_FILE = myapp.rc

RESOURCES += \
    appresources.qrc


