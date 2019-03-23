#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include <QObject>
#include "customwidget.h"
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>

class buttonWidget : public customWidget {
    Q_OBJECT
public:
    explicit buttonWidget(QWidget *parent, QDomElement* domElt = nullptr);
    void buildXml(QDomDocument& doc);

private:
    int maxValRange = 10000;
    int minValRange = 10;
    QPushButton* button;
    QLabel* btnLabelState;
    QLabel* btnLabelRepeat;
    QLabel* btnCheckable;
    QCheckBox* check;
    QCheckBox* autoRepeat;
    void setBackGroundButton(QString img);
    QSpinBox* delaySpinBox;
    QSpinBox* repeatIntervalSpinBox;
    QLabel* delayLabel;
    QLabel* repeatIntervalLabel;

private slots:
    void chkStateChanged(int value);
    void autoRepeatStateChanged(int value);
    void btnPressed();
    void btnReleased();
    void btnClicked(bool t);
    void delayValueChanged(int t);
    void repeatIntervalValueChanged(int t);

};

#endif // BUTTONWIDGET_H
