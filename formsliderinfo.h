#ifndef FORMSLIDERINFO_H
#define FORMSLIDERINFO_H

#include <QWidget>

namespace Ui {
class FormSliderInfo;
}

class FormSliderInfo : public QWidget
{
    Q_OBJECT

public:
    explicit FormSliderInfo(QString name, int rMin, int rMax, int val, QWidget *parent = nullptr);
    ~FormSliderInfo();
    void setValue(int v);
    void setSingleStep(int s);
    void setPageStep(int s);
    void setRange(int rMin, int rMax);
private:
    Ui::FormSliderInfo *ui;
    int value;

private slots:
    void handleValue(int i);
    void valueBoxEditingFinished();

signals:
    void valueChanged(int v);
};

#endif // FORMSLIDERINFO_H
