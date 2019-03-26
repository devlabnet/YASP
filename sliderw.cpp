#include "sliderw.h"
#include "ui_sliderW.h"
#include <QDebug>
#include "widgetsarealayout.h"

SliderW::SliderW(QString name, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::sliderw) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    singleStep = abs(maxValRange - minValRange)*10/2;
    qDebug() << "singleStep " << singleStep;
    ui->minSpin->setRange(minValRange, maxValRange);
    ui->minSpin->setValue(minValRange);
    ui->maxSpin->setRange(minValRange, maxValRange);
    ui->maxSpin->setValue(maxValRange);
    ui->stepSpin->setValue(singleStep/1000);
    value = (maxValRange + minValRange)/2000;
    ui->slider->setValue(value*1000);
    ui->slider->setRange(minValRange * 1000, maxValRange * 1000);
    ui->slider->setSingleStep(singleStep);
    ui->ValueLine->setText(QString::number(ui->slider->value(),'f', 2));
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
    connect(ui->slider, SIGNAL(valueChanged(int)),this, SLOT(slideValueChanged(int)));
    connect(ui->stepSpin, SIGNAL(valueChanged(double)),this, SLOT(slideStepChanged(double)));
}

SliderW::~SliderW() {
    delete ui;
}

void SliderW::slideValueChanged(int v) {
    value = v/1000;
    ui->ValueLine->setText(QString::number(value, 'f', 2));
}

void SliderW::slideStepChanged(double v) {
    qDebug() << "slideStepChanged " << v;
    singleStep = (int)v*1000;
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
}

