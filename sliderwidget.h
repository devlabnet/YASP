#ifndef CMDWIDGET_H
#define CMDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include "customwidget.h"

class sliderWidget : public customWidget
{
    Q_OBJECT
public:
    explicit sliderWidget(QWidget *parent, QDomElement* domElt = nullptr);
    void buildXml(QDomDocument& doc);

private:
    int maxValRange = 1000;
    int minValRange = -1000;
    int tickInterval = 10;
    QLabel* valLabelBox1;
    QSlider *slide;
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
};

#endif // CMDWIDGET_H
