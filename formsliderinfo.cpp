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
    ui->slider->setRange(rMin, rMax);
}

void FormSliderInfo::handleValue(int i) {
    qDebug() << "Value: " << i;
    value = i;
    ui->value->setText(QString::number(value));
    emit valueChanged(i);
}

void FormSliderInfo::valueBoxEditingFinished() {
    ui->slider->setValue(ui->value->text().toInt());
}
