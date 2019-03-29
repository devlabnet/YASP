#include "widgetswindow.h"
#include "ui_widgetswindow.h"
#include "sliderw.h"
#include "dialw.h"
#include "buttonw.h"
#include "siggenw.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

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
//        qDebug() << "MSG Sent : " << msg;
        emit messageSent(msg);
    }
}

void widgetsWindow::on_actionSave_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Widgets File"), "",
                                                    tr("Widgets File (*.xml);"));
    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        QTextStream stream( &file );
        QDomDocument doc("WIDGETS");
        QDomElement root = doc.createElement("WIDGETS");
        doc.appendChild(root);
        widgetsLayout->saveXml(doc);
        doc.save(stream, QDomNode::EncodingFromDocument);
    }
}

// XML Section
bool widgetsWindow::openXml() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Widgets File"), "", tr("Xml (*.xml)"));
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::information(this, tr("Unable to read file"),
            file.errorString());
        return false;
    }
    bool res = xmlWidgets.setContent(&file);
    file.close();
    return res;
}

void widgetsWindow::createWidget(QString type, QDomElement* domElt) {
    qDebug() << "Add Widget : " << type;
    if (type == "Dial") {
        dialW* f = new dialW(domElt);
        widgetsLayout->addWidget(f);
    } else if (type == "Slider") {
        SliderW* f = new SliderW(domElt);
        widgetsLayout->addWidget(f);
    } else if (type == "Button") {
        ButtonW* f = new ButtonW(domElt);
        widgetsLayout->addWidget(f);
    } else if (type == "SigGen") {
        siggenW* f = new siggenW(domElt);
        widgetsLayout->addWidget(f);
    }
}

void widgetsWindow::doXml() {
    // remove all widgets
    widgetsLayout->cleanWidgets();
    // _________________________
    // ::: Read the root tag :::
    // Extract the root markup
    QDomElement root = xmlWidgets.documentElement();
    // Get root names and attributes
    QString Type = root.tagName();
    // _________________
    // ::: Read data :::
    // Get the first child of the root (Markup COMPONENT is expected)
    QDomElement Component = root.firstChild().toElement();
    // Loop while there is a child
    while(!Component.isNull()) {
        // Check if the child tag name is WIDGET
        if (Component.tagName() == "WIDGET") {
            // Read and display the Widget ID
            QString type = Component.attribute("TYPE","");
            // Get the first child of the component
            QDomElement Child = Component.firstChild().toElement();
            if (!Child.isNull()) {
                createWidget(type, &Child);
            }
        }
        // Next widget
        Component = Component.nextSibling().toElement();
    }
}

void widgetsWindow::on_actionLoad_triggered() {
    if (openXml()) {
        doXml();
    }
}

void widgetsWindow::on_actionRemove_All_triggered() {
    // remove all widgets
    widgetsLayout->cleanWidgets();
}

void widgetsWindow::on_actionAdd_SliderWidget_triggered() {
    SliderW* f = new SliderW();
    widgetsLayout->addWidget(f);
}

void widgetsWindow::on_actionAdd_Dial_Widget_triggered() {
    dialW* f = new dialW();
    widgetsLayout->addWidget(f);
}

void widgetsWindow::on_actionButton_triggered() {
    ButtonW* f = new ButtonW();
    widgetsLayout->addWidget(f);
}

void widgetsWindow::on_actionSignal_Generator_triggered() {
    siggenW* f = new siggenW();
    widgetsLayout->addWidget(f);
}
