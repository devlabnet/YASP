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
    void setSerialPort(QSerialPort* p) {
        port = p;
    }

private slots:
    void on_actionAdd_SliderWidget_triggered();
    void on_actionAdd_Dial_Widget_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_actionRemove_All_triggered();
    void on_actionButton_triggered();
//    void on_actionSignal_Generator_triggered();

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
//    bool eventFilter(QObject *object, QEvent *event);

};

#endif // MAINWINDOW_H
