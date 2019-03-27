#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "widgetsarealayout.h"
#include <QSerialPort>

namespace Ui {
class widgetsWindow;
}

class widgetsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit widgetsWindow(QSerialPort *p, QWidget *parent = nullptr);
    ~widgetsWindow();
    void sendToPort(QString msg);
private slots:
    void on_actionAdd_SliderWidget_triggered();
    void on_actionAdd_Dial_Widget_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();

signals:
    void messageSent(QString msg);

private:
    Ui::widgetsWindow *ui;
    QSerialPort* port;
    WidgetsAreaLayout *widgetsLayout;
    QDomDocument xmlWidgets;
    void createWidget(QString type, QDomElement* domElt);
    bool openXml();
    void doXml();
};

#endif // MAINWINDOW_H
