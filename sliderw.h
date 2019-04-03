#ifndef SLIDERW_H
#define SLIDERW_H

#include "boxwidget.h"

namespace Ui {
class sliderw;
}

class SliderW : public boxWidget {
    Q_OBJECT

public:
    explicit SliderW(QDomElement* dom = nullptr, boxWidget *parent = nullptr);
    ~SliderW();
signals:

private slots:
    void updateTabSizes(int index);
    void on_cmdLabel_editingFinished();
    void valueBoxEditingFinished();
    void slideValueChanged(int v);
    void slideStepChanged(int v);
    void trackingToggle(bool t);
    void setMinimumSlide(int v);
    void setMaximumSlide(int v);

protected:
//    void enterEvent(QEvent * event);
//    void mouseMoveEvent(QMouseEvent * event);

private:
    Ui::sliderw *ui;
    int maxValRange = 10000;
    int minValRange = -10000;
    int singleStep;
    int value;
    void updateInfo();
    void buildXml(QDomDocument& doc);
};

#endif // SLIDERW_H
