#ifndef FORMSLIDERINFO_H
#define FORMSLIDERINFO_H

#include <QWidget>
#include <QIntValidator>

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
    void extendRange(int rMin, int rMax);
    void setMinimum(int rMin);
    void extendMinimum(int rMin);
    int minimum() const;
    void setMaximum(int rMax);
    void extendMaximum(int rMax);
    int maximum() const;
private:
    Ui::FormSliderInfo *ui;
    int value;
    QIntValidator *validator;
private slots:
    void handleValue(int i);
    void valueBoxEditingFinished();

signals:
    void valueChanged(int v);
};

#endif // FORMSLIDERINFO_H
