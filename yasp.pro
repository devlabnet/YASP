#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T14:53:33
#
#-------------------------------------------------
#include (C:\Qwt-6.1.4\features\qwt.prf)
#DEFINES += QCUSTOMPLOT_USE_OPENGL

QT       += core gui
QT       += serialport
QT += xml
QT += network
QT += webenginewidgets

CONFIG += c++11
CONFIG += QwtWidgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = yasp
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    	qcustomplot.cpp \
    helpwindow.cpp \
    dialogwidgets.cpp \
    sliderwidget.cpp \
    customwidget.cpp \
    dialwidget.cpp \
    formsliderinfo.cpp \
    yaspgraph.cpp \
    customwidget.cpp \
    dialogwidgets.cpp \
    dialwidget.cpp \
    formsliderinfo.cpp \
    helpwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    sliderwidget.cpp \
    yaspgraph.cpp \
    buttonwidget.cpp \
    clickablelabel.cpp \
    widgetsarealayout.cpp \
    dialw.cpp \
    sliderw.cpp \
    boxwidget.cpp \
    widgetswindow.cpp

HEADERS  += mainwindow.hpp \
    		qcustomplot.h \
    helpwindow.hpp \
    dialogwidgets.h \
    sliderwidget.h \
    customwidget.h \
    dialwidget.h \
    formsliderinfo.h \
    tabwidget.h \
    yaspgraph.h \
    customwidget.h \
    dialogwidgets.h \
    dialwidget.h \
    formsliderinfo.h \
    helpwindow.hpp \
    mainwindow.hpp \
    qcustomplot.h \
    sliderwidget.h \
    tabwidget.h \
    yaspgraph.h \
    buttonwidget.h \
    clickablelabel.h \
    widgetsarealayout.h \
    sliderw.h \
    dialw.h \
    boxwidget.h \
    widgetswindow.h

FORMS    += mainwindow.ui \
    helpwindow.ui \
    dialogwidgets.ui \
    formsliderinfo.ui \
    widgetswindow.ui \
    dialW.ui \
    sliderW.ui

RESOURCES += \
    appresources.qrc \
    qdarkstyle/style.qrc \

DISTFILES += \
    README.md \
    installer/updates.json \
    installer/readMe.txt \
    installer/YASP_Setup.exe \
    installer/yasp.iss \
    installer/1.bmp \
    yaspLib/yasplib.cpp \
    yaspLib/yasplib.h \
    yaspLib/keywords.txt \
    yaspLib/keywords.txt \
    yaspLib/library.properties \
    yaspLib/examples/plotTest/plotTest.ino \
    yaspLib/examples/modulation/modulation.ino \
    yaspLib/examples/widget/widget.ino \
    yaspLib/examples/simple/simple.ino \
    yaclLib/yaclLib.cpp \
    yaclLib/yaclLib.h \
    yaclLib/keywords.txt \
    yaclLib/library.properties \
    yaclLib/examples/simple/simple.ino \
    yaclLib/examples/calculator/calculator.ino \
    yaclLib/README.md \
    yaspLib/README.md



