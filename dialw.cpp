#include "dialw.h"
#include "ui_dialW.h"
#include <QDebug>
#include "widgetsarealayout.h"

dialW::dialW(QDomElement *dom, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::dialw) {
    ui->setupUi(this);
    ui->label->setText("Dial");
    ui->cmdLabelId->setText("");
    ui->Tracking->setChecked(false);
    ui->dial->setTracking(false);
    ui->dial->setWrapping(false);
    ui->MinVal->setRange(minValRange, maxValRange);
    ui->MaxVal->setRange(minValRange, maxValRange);
    ui->frameDial->setObjectName("frameDial");
    ui->frameDial->setStyleSheet("#frameDial{padding:0px; margin:0px; background-color:rgb(209, 255, 209);border-radius:10px;border:2px solid rgb(80, 200, 80)} ");
    value = (maxValRange + minValRange)/2;
    singleStep = abs(maxValRange - minValRange)/20;
    if (dom != nullptr) {
        QDomElement Child = *dom;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "CMD_ID") {
                id = Child.firstChild().toText().data();
                ui->cmdLabelId->setText(id);
                ui->cmdLabel->setText(id);
                ui->label->setText("Dial " + id);
            }
            if (Child.tagName() == "VALUE") value = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MIN") minValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MAX") maxValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_STEP") singleStep = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "TRACKING") {
                bool track = (Child.firstChild().toText().data() == "1");
                ui->dial->setTracking(track);
                ui->Tracking->setChecked(track);
            }
            if (Child.tagName() == "WRAPPING") {
                bool wrap = (Child.firstChild().toText().data() == "1");
                ui->dial->setWrapping(wrap);
                ui->Wrapping->setChecked(wrap);
            }
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    ui->dial->setRange(minValRange, maxValRange);
    singleStep = abs(maxValRange - minValRange)/20;
    ui->MinVal->setValue(minValRange);
    ui->MaxVal->setValue(maxValRange);
    ui->StepVal->setRange(1, singleStep*10);
    ui->StepVal->setValue(singleStep);
    ui->dial->setValue(value);
    ui->dial->setSingleStep(singleStep);
    ui->ValueLine->setText(QString::number(value));
    updateInfo();
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
    connect(ui->dial, SIGNAL(valueChanged(int)),this, SLOT(slideValueChanged(int)));
    connect(ui->StepVal, SIGNAL(valueChanged(int)),this, SLOT(slideStepChanged(int)));
    connect(ui->Tracking, SIGNAL(toggled(bool)), this, SLOT(trackingToggle(bool)));
    connect(ui->Wrapping, SIGNAL(toggled(bool)), this, SLOT(wrappingToggle(bool)));
    connect(ui->MinVal, SIGNAL(valueChanged(int)), this, SLOT(setMinimumSlide(int)));
    connect(ui->MaxVal, SIGNAL(valueChanged(int)), this, SLOT(setMaximumSlide(int)));
    connect(ui->ValueLine, SIGNAL(editingFinished()), this, SLOT(valueBoxEditingFinished()));
    updateTabSizes(0);
}

dialW::~dialW() {
    delete ui;
}

QString dialW::getId() {
    return ui->cmdLabel->text();
}

void dialW::buildXml(QDomDocument& doc) {
    qDebug() << "dialW::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Dial");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(ui->cmdLabelId->text()));
    tag = doc.createElement("VALUE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->dial->value())));
    tag = doc.createElement("VALUE_MIN");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->MinVal->value())));
    tag = doc.createElement("VALUE_MAX");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->MaxVal->value())));
    tag = doc.createElement("VALUE_STEP");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->StepVal->value())));
    tag = doc.createElement("TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->Tracking->isChecked())));
    tag = doc.createElement("WRAPPING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->Wrapping->isChecked())));
    root.appendChild(widget);
}

void dialW::updateInfo() {
    QString txt = "Min:" + QString::number(ui->dial->minimum())
            + " Max:" + QString::number(ui->dial->maximum())
            + "\n Step:" + QString::number(ui->dial->singleStep());
    if (ui->Tracking->isChecked()) {
        txt += " Track: YES";
    } else {
        txt += " Track: NO";
    }
    if (ui->Wrapping->isChecked()) {
        txt += " Wrap: YES";
    } else {
        txt += " Wrap: NO";
    }
    ui->labelInfo->setText(txt);
}

void dialW::valueBoxEditingFinished() {
    ui->dial->setValue(ui->ValueLine->text().toInt());
}

void dialW::trackingToggle(bool t) {
    ui->dial->setTracking(t);
    updateInfo();
}

void dialW::wrappingToggle(bool t) {
    ui->dial->setWrapping(t);
    updateInfo();
}

void dialW::setMinimumSlide(int v) {
    if (v > ui->dial->maximum()) {
        ui->MinVal->setValue(ui->dial->maximum());
        v = ui->dial->maximum();
    }
    ui->dial->setMinimum(v);
    singleStep = abs(ui->dial->maximum() - ui->dial->minimum())/20;
    ui->StepVal->setValue(singleStep);
//    ui->dial->setSingleStep(singleStep);
//    ui->StepVal->setRange(1, singleStep*10);
//    updateInfo();
}

void dialW::setMaximumSlide(int v) {
    if (v < ui->dial->minimum()) {
        ui->MaxVal->setValue(ui->dial->maximum());
        v = ui->dial->minimum();
    }
    ui->dial->setMaximum(v);
    singleStep = abs(ui->dial->maximum() - ui->dial->minimum())/20;
    ui->StepVal->setValue(singleStep);
//    ui->dial->setSingleStep(singleStep);
//    ui->StepVal->setRange(1, singleStep*10);
//    updateInfo();
}

void dialW::slideValueChanged(int v) {
    value = v;
    ui->ValueLine->setText(QString::number(value));
    QString msg =  ui->cmdLabelId->text() + " " + QString::number(v);
    sendToPort(msg);
}

void dialW::slideStepChanged(int v) {
    singleStep = v;
    ui->dial->setSingleStep(singleStep);
    ui->StepVal->setRange(1, singleStep*10);
    updateInfo();
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
