#ifndef GRAPHCONTAINER_H
#define GRAPHCONTAINER_H

#include <QWidget>
#include "qcustomplot.h"

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

signals:

public slots:
};

#endif // GRAPHCONTAINER_H
