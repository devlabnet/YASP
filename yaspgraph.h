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

private:
    int id;
    QCPGraph* graph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
};

#endif // YASPGRAPH_H
