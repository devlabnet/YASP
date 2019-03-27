#include "dialw.h"
#include "ui_dialW.h"
#include <QDebug>
#include "widgetsarealayout.h"

dialW::dialW(QDomElement *dom, boxWidget *parent) : boxWidget(parent), ui(new Ui::dialw) {
    ui->setupUi(this);
    ui->label->setText("Dial");
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
}

dialW::~dialW() {
    delete ui;
}

QString dialW::getId() {
    return ui->cmdLabel->text();
}

void dialW::buildXml(QDomDocument& doc) {
//    customWidget::buildXml(doc);
    qDebug() << "sliderWidget::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement r = doc.firstChildElement("00");
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Dial");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(ui->labelId->text()));
//    root.appendChild(widget);
//    tag = doc.createElement("VALUE_MIN");
//    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->minSpin->value())));
//    root.appendChild(widget);
//    tag = doc.createElement("VALUE_MAX");
//    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->maxSpin->value())));
//    root.appendChild(widget);
//    tag = doc.createElement("VALUE_TICK");
//    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->slider->tickInterval())));
    tag = doc.createElement("TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->Tracking->isChecked())));
    root.appendChild(widget);
}

void dialW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void dialW::on_cmdLabel_editingFinished() {
    // Check for duplicate
    QString newId = ui->cmdLabel->text();
    if (checkId(newId)) {
        id = newId;
        ui->tabBox->setTabEnabled(0, true);
    } else {
        // restore last id
        ui->cmdLabel->setText(id);
        if (id.isEmpty()) {
            ui->tabBox->setTabEnabled(0, false);
        }
    }
    ui->cmdLabelId->setText(id);
    ui->label->setText("Dial " + id);
}
