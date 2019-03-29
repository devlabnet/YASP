#ifndef SIGGENW_H
#define SIGGENW_H

#include "boxwidget.h"
#include <QIntValidator>

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
//    void freqSpinPressed();
    void slideMoveOk();

private:
    Ui::siggenw *ui;
    int minFreqRange = 1;
    int maxFreqRange = 10000;
    int sampleRate = 100000;
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

};

#endif // SIGGENW_H
