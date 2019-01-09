#ifndef DIALWIDGET_H
#define DIALWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDial>
#include <QSpinBox>
#include "customwidget.h"

class dialwidget : public customWidget
{
    Q_OBJECT
public:
    explicit dialwidget(QWidget *parent, QDomElement* domElt = nullptr);
    void buildXml(QDomDocument& doc);

private:
    int maxValRange = 1000;
    int minValRange = -1000;
    int tickInterval = 10;
    QLabel* valLabelBox1;
    QLineEdit* valueBox;
    QDial *dial;
    QSpinBox* minimumSpinBox;
    QSpinBox* maximumSpinBox;
    QLabel* minLabelBox1;
    QLabel* maxLabelBox1;
    QLabel* minValLabelBox1;
    QLabel* maxValLabelBox1;

signals:

public slots:

private slots:
    void slideValue(int value);
    void setMinimumSlide(int v);
    void setMaximumSlide(int v);
    void setSingleStepSlide(int v);
    void valueBoxEditingFinished();
};

#endif // DIALWIDGET_H
