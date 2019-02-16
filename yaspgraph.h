#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

class yaspGraph
{
public:
    yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine);
    //-----------------------------------------------------------------------------------------
    QCPItemText* info() {
        return infoText;
    }
    //-----------------------------------------------------------------------------------------
    QCPItemLine* rLine() {
        return refLine;
    }
    //-----------------------------------------------------------------------------------------
    QCPGraph* plot() {
        return infoGraph;
    }
    //-----------------------------------------------------------------------------------------
    void setOffset(int dpn, double o) {
        yOffset = yOffset + o;
        qDebug() << "yOffset " << yOffset << " o-> " << o;
        QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
        for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
            it->value /= yMult;
            it->value += o;
            it->value *= yMult;
//            it->value += o*yMult;

        }
        yAxis /= yMult;
        yAxis += o;
        yAxis *= yMult;
        refLine->start->setCoords(0, yAxis);
        refLine->end->setCoords(dpn, yAxis);
    }
    //-----------------------------------------------------------------------------------------
    double offset() {
        return yOffset;
    }
    //-----------------------------------------------------------------------------------------
    double axis() {
        return yAxis;
    }
    //-----------------------------------------------------------------------------------------
    void setMult(int dpn, double m) {
        yMult = yMult * m;
        qDebug() << "yMult " << yMult << " m-> " << m;
        QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
        for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
            it->value *= m;
        }
        yAxis *= m;
        refLine->start->setCoords(0, yAxis);
        refLine->end->setCoords(dpn, yAxis);
    }
    //-----------------------------------------------------------------------------------------
    double mult() {
        return yMult;
    }
    //-----------------------------------------------------------------------------------------
    void updateRefLine(int dpn) {
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
