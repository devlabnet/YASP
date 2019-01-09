#include "sliderwidget.h"
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSlider>
#include <QTextEdit>
#include <QDebug>
#include <QSpinBox>

sliderWidget::sliderWidget(QWidget *parent, QDomElement *domElt) : customWidget (parent, domElt)
{
    // help Page
    help->setHtml("<h2>Help sliderWidget</h2>");

    if (domElt != nullptr) {
        QDomElement Child = *domElt;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "VALUE_MIN") minValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MAX") maxValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_TICK") tickInterval = Child.firstChild().toText().data().toInt();
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
    slide->setTickPosition(QSlider::TicksBelow);
    slide->setValue(0);
    slide->setRange(minValRange, maxValRange);
    slide->setTickInterval(tickInterval);
    slide->setSingleStep(tickInterval);
    valLabelBox1 = new QLabel("VALUE : " + QString::number(slide->value()));
//    valLabelBox1->setStyleSheet("font-weight: bold; border: 1px solid blue; margin: 2px;");
    valLabelBox1->setAlignment(Qt::AlignHCenter);
    hBoxCommands->addWidget(minLabelBox1);
    hBoxCommands->addWidget(minValLabelBox1);
    hBoxCommands->addWidget(maxLabelBox1);
    hBoxCommands->addWidget(maxValLabelBox1);
    vBoxCommands->addLayout(hBoxCommands);
    vBoxCommands->addWidget(slide);
    vBoxCommands->addWidget(valLabelBox1);

    // Settings Page
    QLabel* minimumLabel = new QLabel("Minimum");
    minimumSpinBox = new QSpinBox();
    minimumSpinBox->setRange(minValRange, maxValRange);
    minimumSpinBox->setValue(minValRange);
    minimumSpinBox->setSingleStep(100);
    QLabel* maximumLabel = new QLabel("Maximum");
    maximumSpinBox = new QSpinBox();
    maximumSpinBox->setRange(minValRange, maxValRange);
    maximumSpinBox->setValue(maxValRange);
    maximumSpinBox->setSingleStep(100);
    QLabel* tickIntervalLabel = new QLabel("Ticks Interval");
    QSpinBox* ticksIntervalSpinBox = new QSpinBox();
    ticksIntervalSpinBox->setRange(0, maxValRange);
    ticksIntervalSpinBox->setValue(tickInterval);
    ticksIntervalSpinBox->setSingleStep(tickInterval);
    controlsLayout->addWidget(minimumLabel, 1, 0);
    controlsLayout->addWidget(minimumSpinBox, 1, 1);
    controlsLayout->addWidget(maximumLabel, 2, 0);
    controlsLayout->addWidget(maximumSpinBox, 2, 1);
    controlsLayout->addWidget(tickIntervalLabel, 3, 0);
    controlsLayout->addWidget(ticksIntervalSpinBox, 3, 1);

    connect(slide, SIGNAL(valueChanged(int)), this, SLOT(slideValue(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMinimumSlide(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximumSlide(int)));
    connect(ticksIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSingleStepSlide(int)));
}

void sliderWidget::slideValue(int value) {
    valLabelBox1->setText("VALUE : " + QString::number(value));
    sendToPort(value);
}

void sliderWidget::setMinimumSlide(int v) {
    if (v > slide->maximum()) {
        minimumSpinBox->setValue(slide->maximum());
        v = slide->maximum();
    }
    slide->setMinimum(v);
    minValLabelBox1->setText(QString::number(v));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setMaximumSlide(int v) {
    if (v < slide->minimum()) {
        maximumSpinBox->setValue(slide->maximum());
        v = slide->minimum();
    }
    slide->setMaximum(v);
    maxValLabelBox1->setText(QString::number(v));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setSingleStepSlide(int v) {
    slide->setTickInterval(v);
    slide->setSingleStep(v);
    if (v == 0) {
        slide->setTickPosition(QSlider::NoTicks);
    } else {
        slide->setTickPosition(QSlider::TicksBelow);
    }
}

void sliderWidget::buildXml(QDomDocument& doc) {
//    customWidget::buildXml(doc);
    qDebug() << "sliderWidget::buildXml";
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Slider");

    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(cmdLabelLine->text()));
    doc.appendChild(widget);

    tag = doc.createElement("VALUE_MIN");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(minimumSpinBox->value())));
    doc.appendChild(widget);
    tag = doc.createElement("VALUE_MAX");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(maximumSpinBox->value())));
    doc.appendChild(widget);
    tag = doc.createElement("VALUE_TICK");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(slide->tickInterval())));
    doc.appendChild(widget);
}



