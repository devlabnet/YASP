#include "buttonwidget.h"
#include <QDebug>

buttonWidget::buttonWidget(QWidget *parent, QDomElement *domElt) : customWidget (parent, domElt) {
    int repeatDelay = 1000;
    int repeatInterval = 1000;
    bool autoRepeatB = false;
    bool toogleB = false;
    if (domElt != nullptr) {
        QDomElement Child = *domElt;
        while (!Child.isNull()) {
            // Read Name and value
            int v = Child.firstChild().toText().data().toInt();
//            qDebug() << "Child.tagName " << Child.tagName() << " val: " << v;
            if (Child.tagName() == "CHECKABLE") toogleB = (v != 0);
            if (Child.tagName() == "REPEAT") autoRepeatB = (v != 0);
            if (Child.tagName() == "DELAY") repeatDelay = v;
            if (Child.tagName() == "INTERVAL") repeatInterval = v;
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }
//    qDebug() << "toogleB " << toogleB;
//    qDebug() << "autoRepeatB " << autoRepeatB;
//    qDebug() << "repeatDelay " << repeatDelay;
//    qDebug() << "repeatInterval " << repeatInterval;
    // command Page
    button = new QPushButton();
//    button->setText("button");
    hBoxCommands->addWidget(button);
    setBackGroundButton("icons8-bouton-urgence-48.png");
    btnCheckable = new QLabel("PUSH");
    btnCheckable->setStyleSheet("font-weight: bold; color: blue");
//    QLabel* bs = new QLabel("STATE");
//    bs->setStyleSheet("font-weight: bold; color: grey;");
    btnLabelState = new QLabel("OFF");
    btnLabelState->setStyleSheet("font-weight: bold; color: green");
    btnLabelRepeat = new QLabel("No Repeat");
    btnLabelRepeat->setStyleSheet("font-weight: bold;");
    hBoxInfos->addWidget(btnCheckable);
//    hBoxInfos->addWidget(bs);
    hBoxInfos->addWidget(btnLabelRepeat);
    hBoxInfos->addWidget(btnLabelState);
    connect(button, SIGNAL(pressed()), this, SLOT(btnPressed()));
    connect(button, SIGNAL(released()), this, SLOT(btnReleased()));
    connect(button, SIGNAL(clicked(bool)), this, SLOT(btnClicked(bool)));

    // Settings Page
    check =  new QCheckBox("Checkable") ;
    check->setStyleSheet("font-weight: bold;");
    autoRepeat = new QCheckBox("autoRepeat");
    autoRepeat->setStyleSheet("font-weight: bold;");
    delayLabel = new QLabel("Delay");
    delayLabel->setStyleSheet("font-weight: bold;");
    delaySpinBox = new QSpinBox();
    delaySpinBox->setRange(minValRange, maxValRange);
    delaySpinBox->setSingleStep(100);
    repeatIntervalLabel = new QLabel("Repeat Interval");
    repeatIntervalLabel->setStyleSheet("font-weight: bold;");
    repeatIntervalSpinBox = new QSpinBox();
    repeatIntervalSpinBox->setRange(minValRange, maxValRange);
    repeatIntervalSpinBox->setSingleStep(100);
    delayLabel->setVisible(false);
    delaySpinBox->setVisible(false);
    repeatIntervalLabel->setVisible(false);
    repeatIntervalSpinBox->setVisible(false);
    controlsLayout->addWidget(check, 1, 0);
    controlsLayout->addWidget(autoRepeat, 1, 1);
    controlsLayout->addWidget(delayLabel, 2, 0);
    controlsLayout->addWidget(delaySpinBox, 2, 1);
    controlsLayout->addWidget(repeatIntervalLabel, 3, 0);
    controlsLayout->addWidget(repeatIntervalSpinBox, 3, 1);
    connect(check, SIGNAL(stateChanged(int)), this, SLOT(chkStateChanged(int)));
    connect(autoRepeat, SIGNAL(stateChanged(int)), this, SLOT(autoRepeatStateChanged(int)));
    connect(delaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(delayValueChanged(int)));
    connect(repeatIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(repeatIntervalValueChanged(int)));
    check->setChecked(toogleB);
    button->setAutoRepeat(autoRepeatB);
    button->setCheckable(toogleB);
    autoRepeat->setChecked(autoRepeatB);
    delaySpinBox->setValue(repeatDelay);
    button->setAutoRepeatDelay(repeatDelay);
    repeatIntervalSpinBox->setValue(repeatInterval);
    button->setAutoRepeatInterval(repeatInterval);
    chkStateChanged(toogleB);
}


void buttonWidget::setBackGroundButton(QString img) {
    button->setStyleSheet(" QPushButton {background-image: url(:/Icons/Icons/" + img + ");\
                            background-color: #eeeeff; \
                            height: 50px; \
                            background-position: center; \
                            background-repeat: no-repeat; \
                            }");
//text-align: right;
}

void buttonWidget::delayValueChanged(int t) {
//    qDebug() << "delayValueChanged " << delaySpinBox->value();
    if (autoRepeat->isChecked()) {
        button->setAutoRepeatDelay(t);
        btnLabelRepeat->setText("Repeat(" + QString::number(delaySpinBox->value())
                                + "/" + QString::number(repeatIntervalSpinBox->value()) + ")");
    }
}

void buttonWidget::repeatIntervalValueChanged(int t) {
//    qDebug() << "repeatIntervalValueChanged " << repeatIntervalSpinBox->value();
    if (autoRepeat->isChecked()) {
        button->setAutoRepeatInterval(t);
        btnLabelRepeat->setText("Repeat(" + QString::number(delaySpinBox->value())
                                + "/" + QString::number(repeatIntervalSpinBox->value()) + ")");
    }
}

void buttonWidget::chkStateChanged(int value) {
//    qDebug() << "chkStateChanged " << value;
    if (value == Qt::Unchecked) {
        btnCheckable->setText("PUSH");
        button->setAutoRepeat(false);
        setBackGroundButton("icons8-bouton-urgence-48.png");
        button->setChecked(false);
        autoRepeat->setVisible(false);
        button->setCheckable(false);
        delayLabel->setVisible(false);
        delaySpinBox->setVisible(false);
        repeatIntervalLabel->setVisible(false);
        repeatIntervalSpinBox->setVisible(false);
        btnLabelState->setText("OFF");
        btnLabelState->setStyleSheet("font-weight: bold; color: green");
    } else {
        btnCheckable->setText("TOGGLE");
        setBackGroundButton("icons8-toggle-off-48.png");
        button->setChecked(false);
        autoRepeat->setVisible(true);
        delayLabel->setVisible(true);
        delaySpinBox->setVisible(true);
        repeatIntervalLabel->setVisible(true);
        repeatIntervalSpinBox->setVisible(true);
        button->setCheckable(true);
        btnLabelState->setText("OFF");
        btnLabelState->setStyleSheet("font-weight: bold; color: green");
    }
    adjustSize();

}

void buttonWidget::autoRepeatStateChanged(int value) {
    if (value == Qt::Unchecked) {
        btnLabelRepeat->setText("No Repeat");
        button->setAutoRepeat(false);
    } else {
        btnLabelRepeat->setText("Repeat(" + QString::number(delaySpinBox->value())
                                + "/" + QString::number(repeatIntervalSpinBox->value()) + ")");
        delayLabel->setVisible(true);
        delaySpinBox->setVisible(true);
        repeatIntervalLabel->setVisible(true);
        repeatIntervalSpinBox->setVisible(true);
        button->setAutoRepeat(true);
        button->setAutoRepeatDelay(delaySpinBox->value());
        button->setAutoRepeatInterval(repeatIntervalSpinBox->value());
    }
//    show();
    adjustSize();
//    currentTabChanged(0);
//    currentTabChanged(1);
}

void buttonWidget::btnPressed() {
//    qDebug() << "btnPressed ";
    if (!button->isCheckable()) {
        btnLabelState->setText("CLICK");
        btnLabelState->setStyleSheet("font-weight: bold; color: blue");
        setBackGroundButton("icons8-natural-user-interface-2-48.png");
        sendToPort(true);
    }
}

void buttonWidget::btnReleased() {
//    qDebug() << "btnReleased ";
    if (!button->isCheckable()) {
        setBackGroundButton("icons8-bouton-urgence-48.png");
         btnLabelState->setText("OFF");
         btnLabelState->setStyleSheet("font-weight: bold; color: green");
         sendToPort(false);
    }
}

void buttonWidget::btnClicked(bool t) {
//    qDebug() << "btnClicked " << t;
    if (check->isChecked()) {
        if (t) {
            btnLabelState->setText("ON");
            btnLabelState->setStyleSheet("font-weight: bold; color: red");
            setBackGroundButton("icons8-toggle-on-48.png");
        } else {
            btnLabelState->setText("OFF");
            btnLabelState->setStyleSheet("font-weight: bold; color: green");
            setBackGroundButton("icons8-toggle-off-48.png");
        }
        sendToPort(t);
    }
}

void buttonWidget::buildXml(QDomDocument& doc) {
//    qDebug() << "buttonWidget::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement r = doc.firstChildElement("00");
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Button");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(cmdLabelLine->text()));
    root.appendChild(widget);
    tag = doc.createElement("CHECKABLE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(check->isChecked())));
    root.appendChild(widget);
    tag = doc.createElement("DELAY");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(delaySpinBox->value())));
    root.appendChild(widget);
    tag = doc.createElement("INTERVAL");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(repeatIntervalSpinBox->value())));
    root.appendChild(widget);
    tag = doc.createElement("REPEAT");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(autoRepeat->isChecked())));
    root.appendChild(widget);
}
