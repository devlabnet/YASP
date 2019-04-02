#ifndef SIGGENW_H
#define SIGGENW_H

#include "boxwidget.h"
#include <QIntValidator>
#include <QTimer>
#include <QThread>
#include "precisepolling.h"

namespace Ui {
class siggenw;
}

class siggenW : public boxWidget
{
    Q_OBJECT

public:
    explicit siggenW(QDomElement* dom = nullptr, boxWidget *parent = nullptr);
    ~siggenW();
    QString getId();
protected:
private slots:
    void updateTabSizes(int index);
//    void labelMoveClicked(Qt::MouseButton b);
//    void labelDelClicked(Qt::MouseButton b);
    void timerSlot();
    void on_cmdLabel_editingFinished();
    void freqValueChanged(int v);
    void pulseTrackingToggle(bool t);
    void freqTrackingToggle(bool t);
    void pulseToggle(bool b);
    void triangleToggle(bool b);
    void sawToggle(bool b);
    void sinusToggle(bool b);
    void sampleRateChanged(int v);
    void freqValLineChanged();
    void pulsePcValLineChanged();
    void resChanged(const QString& s);
    void pulsePcChanged(int v);
    void on_goBtn_toggled(bool checked);
    void send(QString s);
private:
    enum functionType { Pulse     = 0x00,
                        Triangle  = 0x01,
                        Saw       = 0x02,
                        Sinus     = 0x03
                     };
    Ui::siggenw *ui;
    int minFreqRange = 1;
    const int maxSampleRate = 1000;
    const int maxFreqRange = 1000;
    int sampleRate = maxSampleRate;
    functionType functionMode = functionType::Sinus;
    QTimer genTimer;
    QString mode;
    int resolution = 1024;
    int pulsePerCent = 50;
    int singleStep;
    int value;
    bool moveOk = false;
    QIntValidator* pulsePcValidator;
    QIntValidator* frequencyValidator;
    void updateInfo();
    void buildXml(QDomDocument& doc);
    void checkRadioMode(QString m);
    void updateGenValues();
    int frequency;
    int pulseWidthScaled;
    int period;
    int tickCnt;
    int tCnt;
    int pCnt;
    int valInc;
    int valByte;
    int waveOut;
    qint64 time0;

    QThread thread;
    PrecisePolling pp;

};

#endif // SIGGENW_H
