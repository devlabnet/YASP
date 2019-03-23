#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include "customwidget.h"

#define sliderDivider 1000.0

class sliderWidget : public customWidget {
    Q_OBJECT
public:
    explicit sliderWidget(QWidget *parent, QDomElement* domElt = nullptr);
    void buildXml(QDomDocument& doc);

private:
    double maxValRange = 1000;
    double minValRange = -1000;
    double tickInterval = 10 * sliderDivider;
    bool tracking = false;
    QLabel* valLabelBox1;
    QLineEdit* valueBox;
    QSlider *slide;
    QDoubleSpinBox* minimumSpinBox;
    QDoubleSpinBox* maximumSpinBox;
    QLabel* minLabelBox1;
    QLabel* maxLabelBox1;
    QLabel* minValLabelBox1;
    QLabel* maxValLabelBox1;

signals:

public slots:

private slots:
    void slideValue(int value);
    void trackingToggle(bool t);
    void setMinimumSlide(double v);
    void setMaximumSlide(double v);
    void setSingleStepSlide(double v);
    void valueBoxEditingFinished();
};

#endif // SLIDERWIDGET_H
