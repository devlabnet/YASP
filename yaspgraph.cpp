#include "yaspgraph.h"

yaspGraph::yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine)
    : id(id), infoGraph(g), infoText(info), refLine(rLine) {
    yOffset = 0;
    yMult = 1.0;
   // textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText());
//    dataContainer = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
}
//-----------------------------------------------------------------------------------------
QCPItemText* yaspGraph::info() {
    return infoText;
}
//-----------------------------------------------------------------------------------------
QCPItemLine* yaspGraph::rLine() {
    return refLine;
}
//-----------------------------------------------------------------------------------------
QCPGraph* yaspGraph::plot() {
    return infoGraph;
}
//-----------------------------------------------------------------------------------------
void yaspGraph::save(QTextStream& streamData) {
    QLocale locale = QLocale("fr_FR");
    locale.setNumberOptions(QLocale::OmitGroupSeparator);
    //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    streamData.setLocale(locale);
     // Set Headers
//            streamData << "NAME" << ";" << "TIME" << ";" << "VALUE" << "\n";
    streamData << "TIME" << ";" << "VALUE" << "\n";
    QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
    qDebug() << "SAVE PLOT: " << gData->size();
    for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
        streamData << it->key << ";" << it->value  << "\n";
    }
}
//-----------------------------------------------------------------------------------------
void yaspGraph::setOffset(int dpn, double o) {
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
double yaspGraph::offset() {
    return yOffset;
}
//-----------------------------------------------------------------------------------------
void yaspGraph::setMult(int dpn, double m) {
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
double yaspGraph::mult() {
    return yMult;
}
//-----------------------------------------------------------------------------------------
void yaspGraph::updateRefLine(int dpn) {
    refLine->start->setCoords(0, yOffset);
    refLine->end->setCoords(dpn, yOffset);
}
