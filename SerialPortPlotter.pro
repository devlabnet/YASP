#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T14:53:33
#
#-------------------------------------------------
include (C:\Qwt-6.1.4\features\qwt.prf)

QT       += core gui
QT       += serialport
QT += xml

CONFIG += c++11
CONFIG += QwtWidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SerialPortPlotter
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    	qcustomplot.cpp \
    helpwindow.cpp \
    dialogwidgets.cpp \
    sliderwidget.cpp \
    customwidget.cpp \
    dialwidget.cpp \
    graphcontainer.cpp \
    formsliderinfo.cpp \
    knob.cpp

HEADERS  += mainwindow.hpp \
    		qcustomplot.h \
    helpwindow.hpp \
    dialogwidgets.h \
    sliderwidget.h \
    customwidget.h \
    dialwidget.h \
    graphcontainer.h \
    formsliderinfo.h \
    tabwidget.h \
    knob.h


FORMS    += mainwindow.ui \
    helpwindow.ui \
    dialogwidgets.ui \
    formsliderinfo.ui

RC_FILE = myapp.rc

RESOURCES += \
    appresources.qrc


