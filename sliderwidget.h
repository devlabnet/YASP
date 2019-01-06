#ifndef CMDWIDGET_H
#define CMDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

class sliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit sliderWidget(QWidget *parent = nullptr);

private:
    int maxValRange = 1000;
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
    void deleteRow(QWidget* widget);

public slots:

private slots:
    void ShowContextMenu(const QPoint &pos);
    void deleteWidget();
    void slideValue(int value);

    void setMinimumSlide(int v);
    void setMaximumSlide(int v);
    void setSingleStepSlide(int v);

};

#endif // CMDWIDGET_H
