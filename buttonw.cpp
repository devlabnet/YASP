#include "buttonw.h"
#include "ui_buttonW.h"
#include <QDebug>
#include "widgetsarealayout.h"

ButtonW::ButtonW(QDomElement* dom, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::buttonw) {
    ui->setupUi(this);
    ui->label->setText("Button");
    ui->labelId->setText("");
    ui->AutoRepeat->setChecked(false);
    ui->Checkable->setChecked(false);
    ui->Delay->setVisible(false);
    ui->DelayVal->setVisible(false);
    ui->Repeat->setVisible(false);
    ui->RepeatVal->setVisible(false);
    ui->AutoRepeat->setVisible(false);
    setBackGroundButton("icons8-bouton-urgence-48.png");
    if (dom != nullptr) {
        QDomElement Child = *dom;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "CMD_ID") {
                id = Child.firstChild().toText().data();
                ui->labelId->setText(id);
                ui->cmdLabel->setText(id);
                ui->label->setText("Button " + id);
            }
            if (Child.tagName() == "DELAY") delayVal = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "REPEAT") repeatVal = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "CHECKABLE") {
                bool check = (Child.firstChild().toText().data() == "1");
                ui->pushButton->setCheckable(check);
                ui->Checkable->setChecked(check);
            }
            if (Child.tagName() == "AUTOREPEAT") {
                bool rep = (Child.firstChild().toText().data() == "1");
                ui->pushButton->setAutoRepeat(rep);
                ui->AutoRepeat->setChecked(rep);
            }
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    ui->DelayVal->setRange(1, maxDelay);
    ui->DelayVal->setValue(delayVal);
    ui->RepeatVal->setRange(1, maxRepeat);
    ui->RepeatVal->setValue(repeatVal);
    updateInfo();
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
    connect(ui->DelayVal, SIGNAL(valueChanged(int)),this, SLOT(delayChanged(int)));
    connect(ui->RepeatVal, SIGNAL(valueChanged(int)),this, SLOT(repeatChanged(int)));
    connect(ui->Checkable, SIGNAL(toggled(bool)), this, SLOT(checkableToggle(bool)));
    connect(ui->AutoRepeat, SIGNAL(toggled(bool)), this, SLOT(autoRepeatToggle(bool)));
    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(btnPressed()));
    connect(ui->pushButton, SIGNAL(released()), this, SLOT(btnReleased()));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(btnClicked(bool)));
    updateTabSizes(0);
}

ButtonW::~ButtonW() {
    delete ui;
}

void ButtonW::buildXml(QDomDocument& doc) {
    qDebug() << "ButtonW::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "Button");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(ui->labelId->text()));
//    root.appendChild(widget);
    tag = doc.createElement("CHECKABLE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->Checkable->isChecked())));
//    root.appendChild(widget);
    tag = doc.createElement("AUTOREPEAT");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->AutoRepeat->isChecked())));
//    root.appendChild(widget);
    tag = doc.createElement("DELAY");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->DelayVal->value())));
    tag = doc.createElement("REPEAT");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->RepeatVal->value())));
    root.appendChild(widget);
}

void ButtonW::setBackGroundButton(QString img) {
    ui->pushButton->setStyleSheet(" QPushButton {background-image: url(:/Icons/Icons/" + img + ");\
                            background-color: #eeeeff; \
                            height: 50px; \
                            background-position: center; \
                            background-repeat: no-repeat; \
                            }");
}

void ButtonW::updateInfo() {
    QString txt = "";
    if (ui->Checkable->isChecked()) {
        txt += "TOGGLE";
        if (ui->pushButton->isChecked()) {
            setBackGroundButton("icons8-toggle-on-48.png");
        } else {
            setBackGroundButton("icons8-toggle-off-48.png");
        }
    } else {
        txt += "PUSH";
        setBackGroundButton("icons8-bouton-urgence-48.png");
    }
    if (ui->AutoRepeat->isChecked()) {
        ui->pushButton->setAutoRepeat(true);
        txt += " AUTO REPEAT";
        txt += "\nDel: ";
        txt += QString::number(ui->DelayVal->value());
        txt += " Rep: ";
        txt += QString::number(ui->RepeatVal->value());
    } else {
        ui->pushButton->setAutoRepeat(false);
        txt += " NO AUTO REPEAT";
    }
    ui->labelInfo->setText(txt);
}

void ButtonW::delayChanged(int v) {
    ui->pushButton->setAutoRepeatDelay(v);
    updateInfo();
}

void ButtonW::repeatChanged(int v) {
    ui->pushButton->setAutoRepeatInterval(v);
    updateInfo();
}

void ButtonW::checkableToggle(bool b) {
    ui->pushButton->setCheckable(b);
    ui->pushButton->setChecked(false);
    ui->AutoRepeat->setVisible(b);
    if (!b) {
        ui->AutoRepeat->setChecked(false);
    }
    updateInfo();
}

void ButtonW::autoRepeatToggle(bool b) {
    ui->Delay->setVisible(b);
    ui->DelayVal->setVisible(b);
    ui->Repeat->setVisible(b);
    ui->RepeatVal->setVisible(b);
    updateInfo();
}

void ButtonW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void ButtonW::on_cmdLabel_editingFinished() {
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
    ui->labelId->setText(id);
    ui->label->setText("Slider " + id);
}

void ButtonW::btnPressed() {
//    qDebug() << "btnPressed ";
    if (!ui->pushButton->isCheckable()) {
        setBackGroundButton("icons8-natural-user-interface-2-48.png");
        QString msg =  ui->cmdLabel->text() + " " + QString::number(true);
        sendToPort(msg);
    }
}

void ButtonW::btnReleased() {
//    qDebug() << "btnReleased ";
    if (!ui->pushButton->isCheckable()) {
        setBackGroundButton("icons8-bouton-urgence-48.png");
        QString msg =  ui->cmdLabel->text() + " " + QString::number(false);
        sendToPort(msg);
    }
}

void ButtonW::btnClicked(bool t) {
//    qDebug() << "btnClicked " << t;
    if (ui->Checkable->isChecked()) {
        if (t) {
            setBackGroundButton("icons8-toggle-on-48.png");
        } else {
            setBackGroundButton("icons8-toggle-off-48.png");
        }
        QString msg =  ui->cmdLabel->text() + " " + QString::number(t);
        sendToPort(msg);
    }
}

