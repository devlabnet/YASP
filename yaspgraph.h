#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

class yaspGraph {
public:
    yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine);
    QCPItemText* info();
    QCPItemLine* rLine();
    QCPGraph* plot();
    void save(QTextStream& streamData);
    void setOffset(int dpn, double o);
    double offset();
    void setMult(int dpn, double m);
    double mult();
    void updateRefLine(int dpn);
private:
    int id;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
    double yOffset;
    double yMult;
};

#endif // YASPGRAPH_H
