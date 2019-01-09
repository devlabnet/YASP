#include "dialogwidgets.h"
#include "ui_dialogwidgets.h"
#include <QDebug>
#include <QDial>
#include <QSlider>
#include "sliderwidget.h"
#include <QFileDialog>
#include <QMessageBox>

DialogWidgets::DialogWidgets(QSerialPort* p, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWidgets),
    port(p)
{
    ui->setupUi(this);
    ui->tableWidgets->setColumnCount(1);
    ui->tableWidgets->setRowCount(0);
    ui->tableWidgets->setShowGrid(true);
    ui->tableWidgets->horizontalHeader()->hide();
    ui->tableWidgets->verticalHeader()->hide();
    ui->tableWidgets->resizeRowsToContents();
    ui->tableWidgets->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgets->setStyleSheet("QTableWidget::item { border: 2px solid black; background-color: lavender;}");
    setMinimumSize(700, 500);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));

}

DialogWidgets::~DialogWidgets()
{
    delete ui;
}

void DialogWidgets::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);
    QAction actionSave("Save Widgets", this);
    connect(&actionSave, SIGNAL(triggered()), this, SLOT(saveWidgets()));
    contextMenu.addAction(&actionSave);
    QAction actionLoad("Load Widgets", this);
    connect(&actionLoad, SIGNAL(triggered()), this, SLOT(loadWidgets()));
    contextMenu.addAction(&actionLoad);
    contextMenu.exec(mapToGlobal(pos));
}

void DialogWidgets::saveWidgets() {
    saveXml();
}

void DialogWidgets::loadWidgets() {
    if (openXml()) {
        doXml();
    }
}

QTableWidget* DialogWidgets::getTableWidget() {
    return ui->tableWidgets;
}

void DialogWidgets::createWidget(QString type, QDomElement* domElt) {
    QWidget* widget = nullptr;
    qDebug() << "Add Widget : " << type;
    if (type == "Dial") {
        widget = new sliderWidget(this, domElt);
        widget->show();
    } else if (type == "Slider") {
        widget = new sliderWidget(this, domElt);
        widget->show();
    }
    if (widget != nullptr) {
        int rCount = ui->tableWidgets->rowCount();
        ui->tableWidgets->setRowCount(rCount + 1);
        ui->tableWidgets->setCellWidget(rCount, 0, widget);
        ui->tableWidgets->resizeRowsToContents();
    }
}

void DialogWidgets::on_addWidgetBtn_clicked()
{
    QString type = ui->comboBoxWidgets->currentText();
    createWidget(type);
}

void DialogWidgets::on_comboBoxWidgets_activated(const QString &arg1)
{
    qDebug() << "on_comboBoxWidgets_activated : " << arg1;
}

void DialogWidgets::sendToPort(QString msg) {
    if (msg.isEmpty()) return; // Nothing to send
    msg += "\n";
    // Send data
    qint64 result = port->write(msg.toLocal8Bit());
    if (result != -1) {
        // If data was sent successfully, clear line edit
        //ui->receiveTerminal->appendHtml("&rarr;&nbsp;");
        qDebug() << "MSG Sent : " << msg;
    }
}

// XML Section
bool DialogWidgets::openXml() {
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

QDomDocument DialogWidgets::buildXml() {
    QDomDocument doc("WIDGETS");
    QDomElement widget = doc.createElement("WIDGET");
//    widget.setAttribute("TYPE", "Slider");
////    doc.appendChild(widget);
//    QDomElement tag = doc.createElement("CMD_ID");
//    widget.appendChild(tag);
//    QDomText val = doc.createTextNode("x");
//    tag.appendChild(val);

    for (int i = 0; i < ui->tableWidgets->rowCount(); i++) {
        customWidget* cw = dynamic_cast<customWidget*>(ui->tableWidgets->cellWidget(i, 0));
        cw->buildXml(doc);
    }

    return  doc;
}

void DialogWidgets::saveXml() {
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
        QDomDocument doc = buildXml();
        doc.save(stream, QDomNode::EncodingFromDocument);
        //xmlWidgets.save(stream, QDomNode::EncodingFromDocument);
    }
}

void DialogWidgets::cleanWidgets() {
    for (int i = ui->tableWidgets->rowCount()-1; i >= 0; i--) {
        ui->tableWidgets->removeRow(i);
    }
}

void DialogWidgets::doXml() {
    // remove all widgets
    cleanWidgets();
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
