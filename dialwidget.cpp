#include "dialwidget.h"
#include "dialwidget.h"
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSlider>
#include <QTextEdit>
#include <QDebug>
#include <QSpinBox>

dialwidget::dialwidget(QWidget *parent, QDomElement *domElt) : customWidget (parent, domElt) {
//    // help Page
//    help->setHtml("<h2>Help dialwidget</h2>");

    if (domElt != nullptr) {
        QDomElement Child = *domElt;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "VALUE_MIN") minValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_MAX") maxValRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "VALUE_STEP") tickInterval = Child.firstChild().toText().data().toInt();
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
    dial = new QDial();
    dial->setNotchesVisible(true);
    dial->setValue(0);
    dial->setRange(minValRange, maxValRange);
    //dial->setTickInterval(tickInterval);
    dial->setSingleStep(tickInterval);
    valLabelBox1 = new QLabel("VALUE : ");
//    valLabelBox1->setStyleSheet("font-weight: bold; border: 1px solid blue; margin: 2px;");
    valLabelBox1->setAlignment(Qt::AlignHCenter);
    valueBox = new QLineEdit( QString::number(dial->value()));

//    hBoxInfos->addWidget(minLabelBox1);
//    hBoxInfos->addWidget(minValLabelBox1);
//    hBoxInfos->addWidget(maxLabelBox1);
//    hBoxInfos->addWidget(maxValLabelBox1);
//    vBoxCommands->addWidget(dial);
//    vBoxCommands->addWidget(valLabelBox1);

    hBoxInfos->addWidget(minLabelBox1);
    hBoxInfos->addWidget(minValLabelBox1);
    hBoxInfos->addWidget(maxLabelBox1);
    hBoxInfos->addWidget(maxValLabelBox1);
//    vBoxCommands->addLayout(hBoxInfos);
    hBoxCommands->addWidget(dial);
    hBoxValue->addWidget(valLabelBox1);
    hBoxValue->addWidget(valueBox);
//    vBoxCommands->addLayout(hBoxInfos);

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
    QLabel* tickIntervalLabel = new QLabel("Steps Interval");
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

    connect(dial, SIGNAL(valueChanged(int)), this, SLOT(slideValue(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMinimumSlide(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximumSlide(int)));
    connect(ticksIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSingleStepSlide(int)));
    connect(valueBox, SIGNAL(editingFinished()), this, SLOT(valueBoxEditingFinished()));
}

void dialwidget::slideValue(int value) {
//    valLabelBox1->setText("VALUE : " + QString::number(value));
    valueBox->setText(QString::number(value));
    sendToPort(value);
}

void dialwidget::setMinimumSlide(int v) {
    if (v > dial->maximum()) {
        minimumSpinBox->setValue(dial->maximum());
        v = dial->maximum();
    }
    dial->setMinimum(v);
    minValLabelBox1->setText(QString::number(v));
    dial->setValue((dial->maximum() + dial->minimum()) / 2);
}

void dialwidget::setMaximumSlide(int v) {
    if (v < dial->minimum()) {
        maximumSpinBox->setValue(dial->maximum());
        v = dial->minimum();
    }
    dial->setMaximum(v);
    maxValLabelBox1->setText(QString::number(v));
    dial->setValue((dial->maximum() + dial->minimum()) / 2);
}

void dialwidget::setSingleStepSlide(int v) {
//    dial->setTickInterval(v);
    dial->setSingleStep(v);
    if (v == 0) {
        dial->setNotchesVisible(true);
//        dial->setTickPosition(QSlider::NoTicks);
    } else {
        dial->setNotchesVisible(true);
//        dial->setTickPosition(QSlider::TicksBelow);
    }
}

void dialwidget::valueBoxEditingFinished() {
    dial->setValue(valueBox->text().toInt());
}

void dialwidget::buildXml(QDomDocument& doc) {
//    customWidget::buildXml(doc);
    qDebug() << "dialwidget::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Dial");

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
    tag = doc.createElement("VALUE_STEP");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(dial->singleStep())));
    root.appendChild(widget);
}

