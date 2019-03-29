#include "siggenw.h"
#include "ui_siggenW.h"
#include <QDebug>
#include "widgetsarealayout.h"
#include <QTimer>

siggenW::siggenW(QDomElement *dom, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::siggenw) {
    ui->setupUi(this);
    ui->label->setText("SigGen");
    ui->cmdLabelId->setText("");
//    ui->Tracking->setChecked(false);
    value = 100;
    singleStep = 10;
    resolution = 1024;
    pulsePcValidator = new QIntValidator(1, 99);
    frequencyValidator = new QIntValidator(minFreqRange, maxFreqRange);

    ui->freqSpin->setTracking(false);
    ui->freqSpin->setWrapping(false);
    ui->pulseSpin->setRange(1,99);
    ui->pulseSpin->setSingleStep(1);
    ui->pulsePC->setVisible(false);
    ui->pulsePC->setValidator(pulsePcValidator);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);

    ui->sampleRateVal->setRange(100, sampleRate);
    ui->sampleRateVal->setSingleStep(sampleRate/10);
    ui->freqVal->setValidator(frequencyValidator);
//    ui->freqSpin->setStyleSheet("border: 5px solid rgb(255, 0, 0);");
//    ui->frameFreq->setStyleSheet("QFrame{padding:0px; margin:0px; background-color:rgb(255, 255, 102);border-radius:50px;border:2px solid red} ");

    ui->Sinus->setChecked(true);
    mode = "Sinus";
    ui->resVal->addItem("256");
    ui->resVal->addItem("512");
    ui->resVal->addItem("1024");
    ui->resVal->setCurrentIndex(2);
    ui->freqVal->setText(QString::number(value));
    ui->freqSpin->setValue(value);
    if (dom != nullptr) {
        QDomElement Child = *dom;
        while (!Child.isNull()) {
            // Read Name and value
            if (Child.tagName() == "CMD_ID") {
                id = Child.firstChild().toText().data();
                ui->cmdLabelId->setText(id);
                ui->cmdLabel->setText(id);
                ui->label->setText("SigGen " + id);
            }
            if (Child.tagName() == "MODE") mode = Child.firstChild().toText().data();
            if (Child.tagName() == "FREQ_VAL") value = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "SAMPLE_RATE") sampleRate = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "FREQ_MIN") minFreqRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "FREQ_MAX") maxFreqRange = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "RESOLUTION") resolution = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "PULSE_PC") pulsePerCent = Child.firstChild().toText().data().toInt();
            if (Child.tagName() == "FREQ_TRACKING") {
                bool track = (Child.firstChild().toText().data() == "1");
                ui->freqSpin->setTracking(track);
                ui->freqTracking->setChecked(track);
            }
            if (Child.tagName() == "PULSE_TRACKING") {
                bool track = (Child.firstChild().toText().data() == "1");
                ui->pulseSpin->setTracking(track);
                ui->pulseTrack->setChecked(track);
            }
            // Next child
            Child = Child.nextSibling().toElement();
        }
    }

    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    ui->pulseSpin->setValue(pulsePerCent);
    ui->pulsePC->setText(QString::number(pulsePerCent));
    ui->sampleRateVal->setValue((sampleRate));
//    ui->StepVal->setRange(1, singleStep*10);
//    ui->StepVal->setValue(singleStep);
    ui->freqSpin->setValue(value);
    ui->freqSpin->setRange(minFreqRange, maxFreqRange);
    ui->freqSpin->setSingleStep(singleStep);
//    ui->ValueLine->setText(QString::number(value));
//    ui->labelInfo->setText("Min:" + QString::number(minValRange)
//                           + " Max:" + QString::number(minValRange)
//                           + " Track: No");
    updateInfo();
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
    connect(ui->freqSpin, SIGNAL(valueChanged(int)),this, SLOT(freqValueChanged(int)));
//    connect(ui->freqSpin, SIGNAL(sliderPressed()),this, SLOT(freqSpinPressed()));

    connect(ui->pulseTrack, SIGNAL(toggled(bool)), this, SLOT(pulseTrackingToggle(bool)));
    connect(ui->freqTracking, SIGNAL(toggled(bool)), this, SLOT(freqTrackingToggle(bool)));
    connect(ui->Pulse, SIGNAL(toggled(bool)), this, SLOT(pulseToggle(bool)));
    connect(ui->Triangle, SIGNAL(toggled(bool)), this, SLOT(triangleToggle(bool)));
    connect(ui->Saw, SIGNAL(toggled(bool)), this, SLOT(sawToggle(bool)));
    connect(ui->Sinus, SIGNAL(toggled(bool)), this, SLOT(sinusToggle(bool)));
    connect(ui->sampleRateVal, SIGNAL(valueChanged(int)), this, SLOT(sampleRateChanged(int)));
    connect(ui->pulseSpin, SIGNAL(valueChanged(int)), this, SLOT(pulsePcChanged(int)));
    connect(ui->freqVal, SIGNAL(editingFinished()), this, SLOT(freqValLineChanged()));
    connect(ui->pulsePC, SIGNAL(editingFinished()), this, SLOT(pulsePcValLineChanged()));
    connect (ui->resVal, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(resChanged(const QString&)));
    updateTabSizes(0);
}

siggenW::~siggenW() {
    delete ui;
}

QString siggenW::getId() {
    return ui->cmdLabel->text();
}

void siggenW::buildXml(QDomDocument& doc) {
    qDebug() << "siggenW::buildXml";
    QDomNode root = doc.firstChild();
    QDomElement widget = doc.createElement("WIDGET");
    widget.setAttribute("TYPE", "SigGen");
    QDomElement tag = doc.createElement("CMD_ID");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(ui->cmdLabelId->text()));
//    root.appendChild(widget);
    tag = doc.createElement("VALUE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->freqSpin->value())));
    tag = doc.createElement("VALUE_STEP");
    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->StepVal->value())));
//    tag = doc.createElement("TRACKING");
//    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->Tracking->isChecked())));
//    tag = doc.createElement("WRAPPING");
//    widget.appendChild(tag);
//    tag.appendChild(doc.createTextNode(QString::number(ui->Wrapping->isChecked())));
//    root.appendChild(widget);
}

void siggenW::updateInfo() {
//    Min:-1000 Max:1000\nStep:100 Track:NO
    QString txt = "Mode: " + mode;
    txt += "\nSample Rate:" + QString::number(ui->sampleRateVal->value())
            + " Res:" + QString::number(resolution);
    if (ui->freqTracking->isChecked()) {
        txt += "\nFreq Track: YES";
    } else {
        txt += "\nFreq Track: NO";
    }
    if (ui->pulseTrack->isVisible()) {
        if (ui->pulseTrack->isChecked()) {
        txt += " Pulse Track: YES";
        } else {
            txt += " Pulse Track: NO";
        }
    }
    ui->labelInfo->setText(txt);
}

void siggenW::resChanged(const QString& s) {
    resolution = s.toInt();
}

void siggenW::pulseTrackingToggle(bool t) {
    ui->pulseSpin->setTracking(t);
    updateInfo();
}

void siggenW::freqTrackingToggle(bool t) {
    ui->freqSpin->setTracking(t);
    updateInfo();
}

void siggenW::pulseToggle(bool b) {
    Q_UNUSED(b);
    mode = "Pulse";
    ui->pulsePC->setVisible(true);
    ui->framePulse->setVisible(true);
    ui->pulseLabel->setVisible(true);
    ui->pulseSpin->setVisible(true);
    ui->pulseTrack->setVisible(true);
    updateInfo();
}

void siggenW::triangleToggle(bool b) {
    Q_UNUSED(b);
    mode = "Triangle";
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateInfo();
}

void siggenW::sawToggle(bool b) {
    Q_UNUSED(b);
    mode = "Saw";
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateInfo();
}

void siggenW::sinusToggle(bool b) {
    Q_UNUSED(b);
    mode = "Sinus";
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateInfo();
}

void siggenW::sampleRateChanged(int v) {
    sampleRate = v;
    updateInfo();
}

void siggenW::pulsePcChanged(int v) {
   pulsePerCent = v;
   ui->pulsePC->setText(QString::number(pulsePerCent));
}

void siggenW::freqValLineChanged() {
    qDebug() << "freqValLineChanged " << ui->freqVal->text();
    ui->freqSpin->setValue(ui->freqVal->text().toInt());
}

void siggenW::pulsePcValLineChanged() {
    ui->pulseSpin->setValue(ui->pulsePC->text().toInt());
}

void siggenW::slideMoveOk() {
    qDebug() << "slideMoveOk";
//    connect(ui->freqSpin, SIGNAL(sliderMoved(int)),this, SLOT(freqValueChanged(int)));
}

//void siggenW::freqSpinPressed() {
//    qDebug() << "freqSpinPressed";
//    disconnect(ui->freqSpin, SIGNAL(sliderMoved(int)),this, SLOT(freqValueChanged(int)));
//    QTimer::singleShot(500, this, SLOT(slideMoveOk()));
//}

void siggenW::freqValueChanged(int v) {
//    if (moveOk == false) return;
    qDebug() << "freqValueChanged " << v << " str " << QString::number(v);
    ui->freqVal->setText(QString::number(v));
//    QString msg =  ui->cmdLabelId->text() + " " + QString::number(v);
//    sendToPort(msg);
}

void siggenW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void siggenW::on_cmdLabel_editingFinished() {
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
    ui->label->setText("SigGen " + id);
}
