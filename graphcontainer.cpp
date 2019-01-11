#include "graphcontainer.h"
#include <QDebug>

graphContainer::graphContainer(QCPGraph *g, int nop, QWidget *parent) : QWidget(parent), graph(g), NUMBER_OF_POINTS(nop) {
    graph->setPen(QPen(Qt::red));

}

graphContainer::~graphContainer() {
    qDebug() << "graphContainer Destructor";
}

void graphContainer::updateGraph(int pCnt) {
    NUMBER_OF_POINTS = pCnt;
}

void graphContainer::addData(double k, double v) {
    graph->addData(k, v);                 // Add data to Graph 0
    graph->removeDataBefore(k - NUMBER_OF_POINTS);           // Remove data from graph 0
}
