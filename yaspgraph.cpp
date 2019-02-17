#include "yaspgraph.h"

yaspGraph::yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine)
    : id(id), infoGraph(g), infoText(info), refLine(rLine) {
    yOffset = 0;
    yMult = 1.0;
   // textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText());
//    dataContainer = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
}

