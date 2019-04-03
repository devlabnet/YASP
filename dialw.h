#ifndef DIALW_H
#define DIALW_H

#include "boxwidget.h"

namespace Ui {
class dialw;
}

class dialW : public boxWidget
{
    Q_OBJECT

public:
    explicit dialW(QDomElement* dom = nullptr, boxWidget *parent = nullptr);
    ~dialW();
    QString getId();
protected:
private slots:
    void updateTabSizes(int index);
//    void labelMoveClicked(Qt::MouseButton b);
//    void labelDelClicked(Qt::MouseButton b);
    void on_cmdLabel_editingFinished();
    void valueBoxEditingFinished();
    void slideValueChanged(int v);
    void slideStepChanged(int v);
    void trackingToggle(bool t);
    void wrappingToggle(bool t);
    void setMinimumSlide(int v);
    void setMaximumSlide(int v);
private:
    Ui::dialw *ui;
    int maxValRange = 10000;
    int minValRange = -10000;
    int singleStep;
    int value;
    void updateInfo();
    void buildXml(QDomDocument& doc);

};

#endif // DIALW_H
