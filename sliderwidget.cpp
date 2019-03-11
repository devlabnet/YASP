#include "sliderwidget.h"
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSlider>
#include <QTextEdit>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QRadioButton>

sliderWidget::sliderWidget(QWidget *parent, QDomElement *domElt) : customWidget (parent, domElt)
{
//    // help Page
//    help->setHtml("<h2>Help sliderWidget</h2>");

    if (domElt != nullptr) {
        QDomElement Child = *domElt;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "VALUE_MIN") minValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MAX") maxValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_TICK") tickInterval = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "TRACKING") tracking = Child.firstChild().toText().data().toInt();
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }
    // command Page
    minLabelBox1 = new QLabel("MIN");
    minLabelBox1->setStyleSheet("font-weight: bold; color: red");
    maxLabelBox1 = new QLabel("MAX");
    maxLabelBox1->setStyleSheet("font-weight: bold; color: red");
    minValLabelBox1 = new QLabel(QString::number(minValRange));
    maxValLabelBox1 = new QLabel(QString::number((maxValRange)));
    slide = new QSlider(Qt::Horizontal);
    slide->setTracking(tracking);
    slide->setTickPosition(QSlider::TicksBelow);
    slide->setValue(0);
    slide->setRange(minValRange * sliderDivider, maxValRange * sliderDivider);
    slide->setTickInterval(tickInterval);
    slide->setSingleStep(tickInterval);
    valLabelBox1 = new QLabel("VALUE : ");
//    valLabelBox1->setStyleSheet("font-weight: bold; border: 1px solid blue; margin: 2px;");
    valLabelBox1->setAlignment(Qt::AlignHCenter);
    valueBox = new QLineEdit( QString::number(slide->value() / sliderDivider));

    hBoxInfos->addWidget(minLabelBox1);
    hBoxInfos->addWidget(minValLabelBox1);
    hBoxInfos->addWidget(maxLabelBox1);
    hBoxInfos->addWidget(maxValLabelBox1);
//    vBoxCommands->addLayout(hBoxInfos);
    hBoxCommands->addWidget(slide);
    hBoxValue->addWidget(valLabelBox1);
    hBoxValue->addWidget(valueBox);

    // Settings Page
    QLabel* minimumLabel = new QLabel("Minimum");
    minimumSpinBox = new QDoubleSpinBox();
    minimumSpinBox->setRange(minValRange, maxValRange);
    minimumSpinBox->setValue(minValRange);
    minimumSpinBox->setSingleStep(sliderDivider / 10);
    QLabel* maximumLabel = new QLabel("Maximum");
    maximumSpinBox = new QDoubleSpinBox();
    maximumSpinBox->setRange(minValRange, maxValRange);
    maximumSpinBox->setValue(maxValRange);
    maximumSpinBox->setSingleStep(sliderDivider / 10);
    QLabel* tickIntervalLabel = new QLabel("Ticks Interval");
    QDoubleSpinBox* ticksIntervalSpinBox = new QDoubleSpinBox();
    ticksIntervalSpinBox->setRange(0.01, sliderDivider / 10);
    ticksIntervalSpinBox->setDecimals(4);
    ticksIntervalSpinBox->setValue(tickInterval / sliderDivider);
    ticksIntervalSpinBox->setSingleStep(tickInterval / sliderDivider);
    controlsLayout->addWidget(minimumLabel, 1, 0);
    controlsLayout->addWidget(minimumSpinBox, 1, 1);
    controlsLayout->addWidget(maximumLabel, 2, 0);
    controlsLayout->addWidget(maximumSpinBox, 2, 1);
    controlsLayout->addWidget(tickIntervalLabel, 3, 0);
    controlsLayout->addWidget(ticksIntervalSpinBox, 3, 1);
//    QLabel* dividerLabel = new QLabel("Divider");
    QRadioButton* trackingBtn = new QRadioButton("Tracking");
    controlsLayout->addWidget(trackingBtn, 4, 1);
    trackingBtn->setChecked(tracking);
    connect(trackingBtn, SIGNAL(toggled(bool)), this, SLOT(trackingToggle(bool)));
    connect(slide, SIGNAL(valueChanged(int)), this, SLOT(slideValue(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setMinimumSlide(double)));
    connect(maximumSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setMaximumSlide(double)));
    connect(ticksIntervalSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setSingleStepSlide(double)));
    connect(valueBox, SIGNAL(editingFinished()), this, SLOT(valueBoxEditingFinished()));
}

void sliderWidget::trackingToggle(bool t) {
    tracking = t;
    slide->setTracking(t);
}

void sliderWidget::slideValue(int value) {
    valueBox->setText(QString::number(value / sliderDivider));
    sendToPort(value / sliderDivider);
}

void sliderWidget::setMinimumSlide(double v) {
    qDebug() << "setMinimumSlide: " << v;
    v = v * sliderDivider;
    qDebug() << "slideMinVal: " << v;
    if (v > slide->maximum()) {
        minimumSpinBox->setValue(slide->maximum() / sliderDivider);
        v = slide->maximum();
    }
    qDebug() << "slide->setMinimum: " << v;
    slide->setMinimum(v);
    minValLabelBox1->setText(QString::number(v / sliderDivider));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setMaximumSlide(double v) {
    qDebug() << "setMaximumSlide: " << v;
    v = v * sliderDivider;
    qDebug() << "slideMaxVal: " << v;
    if (v < slide->minimum()) {
        maximumSpinBox->setValue(slide->maximum() / sliderDivider);
        v = slide->minimum();
    }
    qDebug() << "slide->setMaximum: " << v;
    slide->setMaximum(v);
    maxValLabelBox1->setText(QString::number(v / sliderDivider));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setSingleStepSlide(double v) {
    v = v * sliderDivider;
    slide->setTickInterval(v);
    slide->setSingleStep(v);
    if (v == 0) {
        slide->setTickPosition(QSlider::NoTicks);
    } else {
        slide->setTickPosition(QSlider::TicksBelow);
    }
}

void sliderWidget::valueBoxEditingFinished() {
    slide->setValue(valueBox->text().toDouble() * sliderDivider);
}

void sliderWidget::buildXml(QDomDocument& doc) {
//    customWidget::buildXml(doc);
    qDebug() << "sliderWidget::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement r = doc.firstChildElement("00");
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Slider");

    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(cmdLabelLine->text()));
    root.appendChild(widget);

    tag = doc.createElement("VALUE_MIN");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(minimumSpinBox->value())));
    root.appendChild(widget);
    tag = doc.createElement("VALUE_MAX");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(maximumSpinBox->value())));
    root.appendChild(widget);
    tag = doc.createElement("VALUE_TICK");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(slide->tickInterval())));
    tag = doc.createElement("TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(tracking)));

    root.appendChild(widget);
}



