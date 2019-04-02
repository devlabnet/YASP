#include "siggenw.h"
#include "ui_siggenW.h"
#include <QDebug>
#include "widgetsarealayout.h"
#include <QTimer>
#include <QDateTime>

siggenW::siggenW(QDomElement *dom, boxWidget *parent) :
    boxWidget(parent),
    ui(new Ui::siggenw) {
    ui->setupUi(this);
    ui->label->setText("SigGen");
    ui->cmdLabelId->setText("");
//    ui->Tracking->setChecked(false);
    value = 10;
    singleStep = 10;
    resolution = 1024;
    pulsePcValidator = new QIntValidator(1, 99);
    frequencyValidator = new QIntValidator(minFreqRange, maxFreqRange);

    ui->freqSpin->setTracking(false);
    ui->freqSpin->setWrapping(false);
    ui->freqTracking->setChecked(false);
    ui->pulseSpin->setRange(1,99);
    ui->pulseSpin->setSingleStep(1);
    ui->pulsePC->setVisible(false);
    ui->pulsePC->setValidator(pulsePcValidator);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseSpin->setTracking(false);
    ui->pulseTrack->setVisible(false);
    ui->pulseTrack->setChecked(false);
    ui->sampleRateVal->setRange(100, sampleRate);
    ui->sampleRateVal->setSingleStep(sampleRate/10);
    ui->freqVal->setValidator(frequencyValidator);
//    ui->freqSpin->setStyleSheet("border: 5px solid rgb(255, 0, 0);");
    ui->frameFreq->setObjectName("frameFreq");
    ui->frameFreq->setStyleSheet("#frameFreq{padding:0px; margin:0px; background-color:rgb(255, 230, 230);border-radius:10px;border:2px solid rgb(255, 100, 100)} ");
    ui->framePulse->setObjectName("framePulse");
    ui->framePulse->setStyleSheet("#framePulse{padding:0px; margin:0px; background-color:rgb(255, 230, 230);border-radius:10px;border:2px solid rgb(255, 100, 100)} ");

    ui->Sinus->setChecked(true);
    mode = "Sinus";
    ui->resVal->addItem("256");
    ui->resVal->addItem("512");
    ui->resVal->addItem("1024");
    ui->resVal->setCurrentIndex(2);
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
//            if (Child.tagName() == "FREQ_MIN") minFreqRange = Child.firstChild().toText().data().toInt();
//            if (Child.tagName() == "FREQ_MAX") maxFreqRange = Child.firstChild().toText().data().toInt();
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
    ui->freqSpin->setSingleStep(singleStep);
    ui->freqSpin->setRange(minFreqRange, sampleRate/4);
    ui->freqSpin->setValue(value);
    tickCnt = maxSampleRate / ui->freqSpin->value();
    ui->freqVal->setText(QString::number(value));
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

    genTimer.setTimerType(Qt::PreciseTimer);
    genTimer.setInterval(1);
    connect(&genTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    checkRadioMode(mode);
    updateTabSizes(0);
    updateGenValues();

//    timer.setInterval(1);
//    timer.setTimerType(Qt::PreciseTimer);
//    QObject::connect(&timer, &QTimer::timeout, &pp, &PrecisePolling::doPolling);
//    timer.start();
//    timer.moveToThread(&thread);
//    pp.moveToThread(&thread);
    connect(&pp, SIGNAL(send(QString )), this, SLOT(send(QString)));
    pp.start(QThread::Priority::TimeCriticalPriority);

}

siggenW::~siggenW() {
    delete ui;
}

void siggenW::send(QString s) {
    sendToPort(s);
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
//    if (Child.tagName() == "MODE") mode = Child.firstChild().toText().data();
//    if (Child.tagName() == "FREQ_VAL") value = Child.firstChild().toText().data().toInt();
//    if (Child.tagName() == "SAMPLE_RATE") sampleRate = Child.firstChild().toText().data().toInt();
//    if (Child.tagName() == "RESOLUTION") resolution = Child.firstChild().toText().data().toInt();
//    if (Child.tagName() == "PULSE_PC") pulsePerCent = Child.firstChild().toText().data().toInt();
//    if (Child.tagName() == "FREQ_TRACKING") {

//    root.appendChild(widget);
    tag = doc.createElement("MODE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(mode));
    tag = doc.createElement("FREQ_VAL");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(ui->freqVal->text()));
    tag = doc.createElement("SAMPLE_RATE");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->sampleRateVal->value())));
    tag = doc.createElement("RESOLUTION");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(resolution)));
    tag = doc.createElement("PULSE_PC");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(pulsePerCent)));
    tag = doc.createElement("FREQ_TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->freqTracking->isChecked())));
    tag = doc.createElement("PULSE_TRACKING");
    widget.appendChild(tag);
    tag.appendChild(doc.createTextNode(QString::number(ui->pulseTrack->isChecked())));
    root.appendChild(widget);
}

void siggenW::checkRadioMode(QString m) {
    if (m == "Pulse") {
        functionMode = functionType::Pulse;
        ui->Pulse->setChecked(true);
    } else if (m == "Triangle") {
        functionMode = functionType::Triangle;
        ui->Triangle->setChecked(true);
    } else if (m == "Saw") {
        functionMode = functionType::Saw;
        ui->Saw->setChecked(true);
    } else { // Sinus
        functionMode = functionType::Sinus;
        ui->Sinus->setChecked(true);
    }
}

void siggenW::updateGenValues() {
    frequency = ui->freqSpin->value();//from 1 to ~50,000
    period = sampleRate/frequency;
    pulseWidthScaled = (period * pulsePerCent)/100;
    tickCnt = maxSampleRate / ui->freqSpin->value();
    switch (functionMode) {
    case functionType::Pulse :
        break;
    case functionType::Triangle :
        valInc = 10;
        break;
    case functionType::Saw :
        valInc = 255 / period;
        break;
    default: // Sinus
        valInc = 20000 / period;
        break;
    }
    qDebug() << "functionMode: " << functionMode;
    qDebug() << "sampleRate: " << sampleRate;
    qDebug() << "frequency: " << frequency;
    qDebug() << "period: " << period;
    qDebug() << "tickCnt: " << tickCnt;
//    qDebug() << "pulsePerCent: " << pulsePerCent;
//    qDebug() << "pulseWidthScaled: " << pulseWidthScaled;
}

void siggenW::updateInfo() {
//    Min:-1000 Max:1000\nStep:100 Track:NO
    QString txt = "Mode: " + mode;
    txt += "  Freq: " + QString::number(ui->freqSpin->value());
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
    updateInfo();
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
    functionMode = functionType::Pulse;
    ui->pulsePC->setVisible(true);
    ui->framePulse->setVisible(true);
    ui->pulseLabel->setVisible(true);
    ui->pulseSpin->setVisible(true);
    ui->pulseTrack->setVisible(true);
    updateGenValues();
    updateInfo();
}

void siggenW::triangleToggle(bool b) {
    Q_UNUSED(b);
    mode = "Triangle";
    functionMode = functionType::Triangle;
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateGenValues();
    updateInfo();
}

void siggenW::sawToggle(bool b) {
    Q_UNUSED(b);
    mode = "Saw";
    functionMode = functionType::Saw;
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateGenValues();
    updateInfo();
}

void siggenW::sinusToggle(bool b) {
    Q_UNUSED(b);
    mode = "Sinus";
    functionMode = functionType::Sinus;
    ui->pulsePC->setVisible(false);
    ui->framePulse->setVisible(false);
    ui->pulseLabel->setVisible(false);
    ui->pulseSpin->setVisible(false);
    ui->pulseTrack->setVisible(false);
    updateGenValues();
    updateInfo();
}

void siggenW::sampleRateChanged(int v) {
    sampleRate = v;
    updateGenValues();
    updateInfo();
}

void siggenW::pulsePcChanged(int v) {
   pulsePerCent = v;
   ui->pulsePC->setText(QString::number(pulsePerCent));
   updateGenValues();
}

void siggenW::freqValLineChanged() {
    qDebug() << "freqValLineChanged " << ui->freqVal->text();
    ui->freqSpin->setValue(ui->freqVal->text().toInt());
}

void siggenW::pulsePcValLineChanged() {
    ui->pulseSpin->setValue(ui->pulsePC->text().toInt());
}

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

void siggenW::timerSlot() {
//    qDebug() << "mode " << functionMode << " -> pCnt: " << pCnt << " / valByte " << valByte;
//    if (pCnt > period) {
//        pCnt = 0;
//    }
//    if (tCnt == 0) {
        switch (functionMode) {
        case functionType::Pulse:
//            if (pulseWidthScaled <= pCnt) {
//                waveOut = 255;
//            } else {
//                waveOut = 0;
//            }
            break;
        case functionType::Triangle:
//            if ((period - pCnt) > pCnt) {
//                if (pCnt == 0) {
//                    valByte = 0;
//                } else {
//                    valByte += valInc;
//                }
//            } else {
//                valByte -= valInc;
//            }
//            qDebug() << " -> valByte " << valByte;

//            if (valByte > 255) {
//                valByte = 255;
//            } else if (valByte < 0) {
//                valByte = 0;
//            }
//            waveOut = valByte;
            break;
        case functionType::Saw:
//            if (tCnt == 0) {
//                waveOut = 0;
//            } else {
                waveOut = resolution * ((float)tCnt / (float)period);
//                qDebug() << "tCnt" <<  tCnt << "tCnt / period" << tCnt / period << " waveOut: " << waveOut;
//            }
            break;
        default:
            break;
        }
//    }
    pCnt++;
    tCnt++;
    if (tCnt >= period) {
        tCnt = 0;
    }
    qDebug() << QDateTime::currentMSecsSinceEpoch() - time0;
    time0 = QDateTime::currentMSecsSinceEpoch();
    QString msg =  ui->cmdLabelId->text() + " " + QString::number(waveOut);
    sendToPort(msg);

}

void siggenW::on_goBtn_toggled(bool checked) {
    qDebug() << "on_goBtn_toggled";



    if (checked) {
        pCnt = 0;
        tCnt = 0;
        valByte = 0;
        waveOut = 0;
//        updateGenValues();
//        genTimer.start();
        pp.StartWork();
    } else {
//        genTimer.stop();
        pp.StopWork();
    }
}
