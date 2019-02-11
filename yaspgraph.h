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
private:
    int id;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
};

#endif // YASPGRAPH_H
