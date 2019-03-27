#include "widgetsWindow.h"
#include "ui_widgetsWindow.h"
#include "sliderw.h"
#include "dialw.h"
#include <QDebug>

widgetsWindow::widgetsWindow(QSerialPort* p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::widgetsWindow), port(p) {
    ui->setupUi(this);
    QWidget* topWidget = new QWidget(this);
    widgetsLayout = new WidgetsAreaLayout();
    topWidget->setLayout(widgetsLayout);
    ui->scrollArea->setWidget(topWidget);
}

widgetsWindow::~widgetsWindow() {
    delete ui;
}

void widgetsWindow::sendToPort(QString msg) {
    if (msg.isEmpty()) return; // Nothing to send
    msg += "\n";
    // Send data
    qint64 result = port->write(msg.toLocal8Bit());
    if (result != -1) {
        // If data was sent successfully, clear line edit
        //ui->receiveTerminal->appendHtml("&rarr;&nbsp;");
        qDebug() << "MSG Sent : " << msg;
        emit messageSent(msg);
    }
}

void widgetsWindow::on_actionAdd_SliderWidget_triggered() {
    SliderW* f = new SliderW("Slider_" + QString::number(widgetsLayout->count()));
    widgetsLayout->addWidget(f);
}

void widgetsWindow::on_actionAdd_Dial_Widget_triggered() {
    dialW* f = new dialW("Dial_" + QString::number(widgetsLayout->count()));
    widgetsLayout->addWidget(f);
}
