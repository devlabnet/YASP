#include "yaspgraph.h"

yaspGraph::yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine)
    : id(id), infoGraph(g), infoText(info), refLine(rLine) {
}