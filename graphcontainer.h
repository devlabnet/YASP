#ifndef GRAPHCONTAINER_H
#define GRAPHCONTAINER_H

#include <QWidget>
#include "qcustomplot.h"
#include <QVBoxLayout>
#include "formsliderinfo.h"

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
    QPen pen;
    QColor penColor;
    QCPItemLine *axisLine;
    int delta;
    int mult;
    FormSliderInfo* slideDelta;
    FormSliderInfo* slideMult;

private slots:
   void handleColor();
   void handleWidth(int i);
   void handleDelta(int i);
   void handleMult(int i);

signals:

public slots:
};

#endif // GRAPHCONTAINER_H
