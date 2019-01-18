#include "formsliderinfo.h"
#include "ui_formsliderinfo.h"
#include <QDebug>

FormSliderInfo::FormSliderInfo(QString name, int rMin, int rMax, int val, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSliderInfo),
    value(val)
{
    ui->setupUi(this);
    ui->label->setText(name);
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    validator = new QIntValidator(rMin, rMax);
    ui->value->setValidator(validator);
    ui->slider->setRange(rMin, rMax);
    ui->slider->setValue(value);
    ui->value->setText(QString::number(value));
    ui->slider->setSingleStep(10);
    ui->slider->setPageStep(100);
    connect(ui->slider, SIGNAL (valueChanged(int)), this, SLOT (handleValue(int)));
    connect(ui->value, SIGNAL(editingFinished()), this, SLOT(valueBoxEditingFinished()));
}

FormSliderInfo::~FormSliderInfo()
{
    delete ui;
}

void FormSliderInfo::setValue(int v) {
    ui->slider->setValue(v);
}

void FormSliderInfo::setSingleStep(int s) {
    ui->slider->setSingleStep(s);
}

void FormSliderInfo::setPageStep(int s) {
    ui->slider->setPageStep(s);
}

void FormSliderInfo::setRange(int rMin, int rMax) {
//    qDebug() << "setRange: " << rMin << " / " << rMax;
    ui->slider->setRange(rMin, rMax);
    validator->setRange(rMin, rMax);
}

void FormSliderInfo::extendRange(int rMin, int rMax) {
    extendMinimum(rMin);
    extendMaximum(rMax);
}

void FormSliderInfo::setMinimum(int rMin)  {
    ui->slider->setMinimum(rMin);
    validator->setBottom(rMin);
}

void FormSliderInfo::extendMinimum(int rMin)  {
    if (rMin < ui->slider->minimum()) {
        ui->slider->setMinimum(rMin);
        validator->setBottom(rMin);
    }
}

int FormSliderInfo::minimum() const  {
    return ui->slider->minimum();
}

void FormSliderInfo::setMaximum(int rMax)  {
    ui->slider->setMaximum(rMax);
    validator->setTop(rMax);
}

void FormSliderInfo::extendMaximum(int rMax)  {
    if (rMax > ui->slider->maximum()) {
        ui->slider->setMaximum(rMax);
        validator->setTop(rMax);
    }
}

int FormSliderInfo::maximum() const  {
    return ui->slider->maximum();
}

void FormSliderInfo::handleValue(int i) {
//    qDebug() << "Value: " << i;
    value = i;
    ui->value->setText(QString::number(value));
    emit valueChanged(i);
}

void FormSliderInfo::valueBoxEditingFinished() {
    ui->slider->setValue(ui->value->text().toInt());
}
