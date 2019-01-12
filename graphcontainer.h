#ifndef GRAPHCONTAINER_H
#define GRAPHCONTAINER_H

#include <QWidget>
#include "qcustomplot.h"
#include <QVBoxLayout>

class graphContainer : public QWidget
{
    Q_OBJECT
public:
    explicit graphContainer(QCPGraph* g, int nop, QWidget *parent = nullptr);
    ~graphContainer();

    QCPGraph* getGraph() { return graph; }
    void addData(double k, double v);
    void clearData();
    void updateGraph(int pCnt);
private:
    QCPGraph* graph;
    int NUMBER_OF_POINTS;                                                                 // Number of points plotted
    QGridLayout* layout;
    QPushButton *colorButton;
    QSlider* deltaSlider;
    QPen pen;
    QColor penColor;
    QCPItemLine *axisLine;
    int delta;

private slots:
   void handleColor();
   void handleWidth(int i);
   void handleDelta(int i);

signals:

public slots:
};

#endif // GRAPHCONTAINER_H
