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
    void save(QTextStream& streamData, QMap<int, int>& pointTime, int nop) {
        QLocale locale = QLocale("fr_FR");
        locale.setNumberOptions(QLocale::OmitGroupSeparator);
        //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        streamData.setLocale(locale);
         // Set Headers
//            streamData << "NAME" << ";" << "TIME" << ";" << "VALUE" << "\n";
        streamData << "TIME" << ";" << "VALUE" << "\n";
        QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
        qDebug() << "SAVE PLOT: " << nop << " /data: " << gData->size() << " /Ticks: " << pointTime.size();
        for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
            if (it->key > nop) return;
//            streamData << pointTime.value(it->key) << ";" << it->value  << "\n";
            streamData << it->key << ";" << it->value  << "\n";
        }
    }
    //-----------------------------------------------------------------------------------------
    void setOffset(int dpn, double o) {
//        qDebug() << "yOffset " << yOffset << " o-> " << o;
        QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
        for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
//            double val = it->value;
//            val += o;
            //            it->value = val;
            it->value += o;
        }
        yOffset += o;
        refLine->start->setCoords(0, yOffset);
        refLine->end->setCoords(dpn, yOffset);
    }
    //-----------------------------------------------------------------------------------------
    double offset() {
        return yOffset;
    }
    //-----------------------------------------------------------------------------------------
    void setMult(int dpn, double m) {
        double nm = yMult * m;
//        qDebug() << "yMult " << yMult << " m-> " << m;
        QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
        for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
            double val = it->value;
            val -= yOffset;
            val /= yMult;
            val *= nm;
            val += yOffset;
            it->value = val;
        }
        yMult = nm;
        refLine->start->setCoords(0, yOffset);
        refLine->end->setCoords(dpn, yOffset);
    }
    //-----------------------------------------------------------------------------------------
    double mult() {
        return yMult;
    }
    //-----------------------------------------------------------------------------------------
    void updateRefLine(int dpn) {
        refLine->start->setCoords(0, yOffset);
        refLine->end->setCoords(dpn, yOffset);
    }
private:
    int id;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
    double yOffset;
    double yMult;
};

#endif // YASPGRAPH_H
