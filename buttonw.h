#ifndef BUTTONW_H
#define BUTTONW_H

#include "boxwidget.h"

namespace Ui {
class buttonw;
}

class ButtonW : public boxWidget {
    Q_OBJECT

public:
    explicit ButtonW(QDomElement* dom = nullptr, boxWidget *parent = nullptr);
    ~ButtonW();
signals:

private slots:
    void updateTabSizes(int index);
    void on_cmdLabel_editingFinished();
    void delayChanged(int v);
    void repeatChanged(int v);
    void checkableToggle(bool b);
    void autoRepeatToggle(bool b);
    void btnPressed();
    void btnReleased();
    void btnClicked(bool t);

protected:
//    void enterEvent(QEvent * event);
//    void mouseMoveEvent(QMouseEvent * event);

private:
    Ui::buttonw *ui;
    int delayVal = 100;
    int repeatVal = 500;
    int maxDelay = 10000;
    int maxRepeat = 10000;
    void setBackGroundButton(QString img);
    void updateInfo();
    void buildXml(QDomDocument& doc);
};

#endif // BUTTONW_H
