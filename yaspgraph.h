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
    void setOffset(int dpn, double o) {
        yOffset = yOffset + o;
        yAxis = refLine->start->coords().y() + o;
        refLine->start->setCoords(0, yAxis);
        refLine->end->setCoords(dpn, yAxis);
    }
    double offset() {
        return yOffset;
    }
    void setMult(double m) {
//        double scale = (double)numDeg / 1000.0;
        yMult = yMult * m;
//        yOffset *= m;
//        yAxis *= m;
//        qDebug() << "scale: " << scale << " mult: " << yMult;
//        if (numDeg < 0) {
//            yMult = yMult * scale;
//        } else {
//            yMult = yMult / scale;
//        }
    }

    double mult() {
        return yMult;
    }
    void update(int dpn) {
        refLine->start->setCoords(0, yAxis);
        refLine->end->setCoords(dpn, yAxis);
    }
private:
    int id;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
    double yOffset;
    double yMult;
    double yAxis;
};

#endif // YASPGRAPH_H
