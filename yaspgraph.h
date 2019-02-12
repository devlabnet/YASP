#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

class yaspGraph
{
public:
    yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine);
    QCPItemText* info() {
        return infoText;
    }
    QCPItemLine* rLine() {
        return refLine;
    }
    QCPGraph* plot() {
        return infoGraph;
    }
    void setOffset(double o) {
        yOffset = o;
    }
    double offset() {
        return yOffset;
    }

private:
    int id;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
    double yOffset = 0;
};

#endif // YASPGRAPH_H
