#include "sliderw.h"
#include "ui_sliderW.h"
#include <QDebug>
#include "widgetsarealayout.h"

SliderW::SliderW(QDomElement* dom, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::sliderw) {
    ui->setupUi(this);
    ui->label->setText("Slider");
    ui->labelId->setText("");
    ui->Tracking->setChecked(false);
    ui->slider->setTracking(false);
    value = (maxValRange + minValRange)/2;
    singleStep = abs(maxValRange - minValRange)/20;
    if (dom != nullptr) {
        QDomElement Child = *dom;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "CMD_ID") {
                id = Child.firstChild().toText().data();
                ui->labelId->setText(id);
                ui->cmdLabel->setText(id);
                ui->label->setText("Slider " + id);
            }
            if (Child.tagName() == "VALUE") value = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MIN") minValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MAX") maxValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_STEP") singleStep = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "TRACKING") {
                bool track = (Child.firstChild().toText().data() == "1");
                ui->slider->setTracking(track);
                ui->Tracking->setChecked(track);
            }
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    qDebug() << "singleStep " << singleStep;
    ui->minSpin->setRange(minValRange, maxValRange);
    ui->minSpin->setValue(minValRange);
    ui->maxSpin->setRange(minValRange, maxValRange);
    ui->maxSpin->setValue(maxValRange);
    ui->stepSpin->setRange(1, singleStep*10);
    ui->stepSpin->setValue(singleStep);
    ui->slider->setValue(value);
    ui->slider->setRange(minValRange, maxValRange);
    ui->slider->setSingleStep(singleStep);
    ui->ValueLine->setText(QString::number(value));
//    ui->labelInfo->setText("Min:" + QString::number(minValRange)
//                           + " Max:" + QString::number(minValRange)
//                           + " Track: No");
    updateInfo();
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
    connect(ui->slider, SIGNAL(valueChanged(int)),this, SLOT(slideValueChanged(int)));
    connect(ui->stepSpin, SIGNAL(valueChanged(int)),this, SLOT(slideStepChanged(int)));
    connect(ui->Tracking, SIGNAL(toggled(bool)), this, SLOT(trackingToggle(bool)));
    connect(ui->minSpin, SIGNAL(valueChanged(int)), this, SLOT(setMinimumSlide(int)));
    connect(ui->maxSpin, SIGNAL(valueChanged(int)), this, SLOT(setMaximumSlide(int)));
    connect(ui->ValueLine, SIGNAL(editingFinished()), this, SLOT(valueBoxEditingFinished()));
}

SliderW::~SliderW() {
    delete ui;
}

void SliderW::buildXml(QDomDocument& doc) {
//    customWidget::buildXml(doc);
    qDebug() << "sliderWidget::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement r = doc.firstChildElement("00");
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Slider");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(ui->labelId->text()));
    root.appendChild(widget);
    tag = doc.createElement("VALUE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->slider->value())));
    root.appendChild(widget);
    tag = doc.createElement("VALUE_MIN");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->minSpin->value())));
    root.appendChild(widget);
    tag = doc.createElement("VALUE_MAX");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->maxSpin->value())));
    root.appendChild(widget);
    tag = doc.createElement("VALUE_STEP");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->stepSpin->value())));
    tag = doc.createElement("TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->Tracking->isChecked())));
    root.appendChild(widget);
}

void SliderW::updateInfo() {
//    Min:-1000 Max:1000\nStep:100 Track:NO
    QString txt = "Min:" + QString::number(ui->slider->minimum())
            + " Max:" + QString::number(ui->slider->maximum())
            + "\n Step:" + QString::number(ui->stepSpin->value())
            + " Track: ";
    if (ui->Tracking->isChecked()) {
        txt += "YES";
    } else {
        txt += "NO";
    }
    ui->labelInfo->setText(txt);
}

void SliderW::valueBoxEditingFinished() {
    ui->slider->setValue(ui->ValueLine->text().toInt());
}

void SliderW::trackingToggle(bool t) {
//    tracking = t;
    ui->slider->setTracking(t);
    updateInfo();
}

void SliderW::setMinimumSlide(int v) {
    qDebug() << "setMinimumSlide: " << v;
    qDebug() << "slideMinVal: " << v;
    if (v > ui->slider->maximum()) {
        ui->minSpin->setValue(ui->slider->maximum());
        v = ui->slider->maximum();
    }
    qDebug() << "slide->setMinimum: " << v;
    ui->slider->setMinimum(v);
//    if (ui->slider->value() < v) {
//        ui->slider->setValue(v);
//    }
//    ui->slider->setValue((ui->slider->maximum() + ui->slider->minimum()) / 2);
    updateInfo();
}

void SliderW::setMaximumSlide(int v) {
    qDebug() << "setMaximumSlide: " << v;
    qDebug() << "slideMaxVal: " << v;
    if (v < ui->slider->minimum()) {
        ui->maxSpin->setValue(ui->slider->maximum());
        v = ui->slider->minimum();
    }
    qDebug() << "slide->setMaximum: " << v;
    ui->slider->setMaximum(v);
//    if (ui->slider->value() > v) {
//        ui->slider->setValue(v);
//    }
//    ui->slider->setValue((ui->slider->maximum() + ui->slider->minimum()) / 2);
    updateInfo();
}

void SliderW::slideValueChanged(int v) {
    value = v;
    ui->ValueLine->setText(QString::number(value));
    QString msg =  ui->labelId->text() + " " + QString::number(v);
    sendToPort(msg);
}

void SliderW::slideStepChanged(int v) {
    qDebug() << "slideStepChanged " << v;
    singleStep = v;
    ui->slider->setSingleStep(singleStep);
}

void SliderW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void SliderW::on_cmdLabel_editingFinished() {
    // Check for duplicate
    QString newId = ui->cmdLabel->text();
    if (checkId(newId)) {
        id = newId;
        ui->tabBox->setTabEnabled(0, true);
//        cmdLabelValue->setText(cmdLabelLine->text());
    } else {
        // restore last id
        ui->cmdLabel->setText(id);
        if (id.isEmpty()) {
            ui->tabBox->setTabEnabled(0, false);
        }
    }
    ui->labelId->setText(id);
    ui->label->setText("Slider " + id);
}

