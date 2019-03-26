#ifndef SLIDERW_H
#define SLIDERW_H

#include "boxwidget.h"

namespace Ui {
class sliderw;
}

class SliderW : public boxWidget {
    Q_OBJECT

public:
    explicit SliderW(QString name, boxWidget *parent = nullptr);
    ~SliderW();
signals:

private slots:
    void updateTabSizes(int index);
    void on_cmdLabel_editingFinished();
    void slideValueChanged(int v);
    void slideStepChanged(double v);

protected:
//    void enterEvent(QEvent * event);
//    void mouseMoveEvent(QMouseEvent * event);

private:
    Ui::sliderw *ui;
    double sliderDivider = 1000;
    double maxValRange = 1000;
    double minValRange = -1000;
    int singleStep;
    double value;
};

#endif // SLIDERW_H
