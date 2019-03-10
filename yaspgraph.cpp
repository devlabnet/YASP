#include "yaspgraph.h"

yaspGraph::yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemStraightLine* rLine, QString plotStr,
                     QColor color, double plotTimeInMilliSeconds)
    : id(id), infoGraph(g), infoText(info), refLine(rLine) {
    yOffset = 0;
    yMult = 1.0;
    plotDashPattern = QVector<qreal>() << 16 << 4 << 8 << 4;
    rLineDashPattern = QVector<qreal>() << 64 << 4 ;
    minY = DBL_MAX;
    maxY = -DBL_MAX;
    QPen pen = QPen(color);
    infoGraph->setPen(pen);
    infoGraph->setName(plotStr);
//    info->layer()->setMode(QCPLayer::lmBuffered);
    info->setColor(color);
    info->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
    info->position->setType(QCPItemPosition::ptAbsolute );
//    info->setSelectable(true);
    info->setPadding(QMargins(2,2,2,2));
    pen.setWidthF(0.5);
    pen.setDashPattern(rLineDashPattern);
//    rLine->layer()->setMode(QCPLayer::lmBuffered);
    rLine->setPen(pen);
//    rLine->setSelectable(false);
    rLine->point1->setCoords(0,0);
    rLine->point2->setCoords(plotTimeInMilliSeconds, 0);
    hidden = false;
   // textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText());
//    dataContainer = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
}

//-----------------------------------------------------------------------------------------
yaspGraph::~yaspGraph() {
     qDebug() << "====================== yaspGraph DESTRUCTOR ======================";
}

//-----------------------------------------------------------------------------------------
void yaspGraph::reset() {
    QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
    for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
        double val = it->value;
        val -= yOffset;
        val /= yMult;
        it->value = val;
    }
    minY -= yOffset;
    minY /= yMult;
    maxY -= yOffset;
    maxY /= yMult;
    yOffset = 0;
    yMult = 1.0;
}


//-----------------------------------------------------------------------------------------
void yaspGraph::hide(bool h) {
    hidden = h;
    infoGraph->setVisible(!h);
    refLine->setVisible(!h);
}

//-----------------------------------------------------------------------------------------
void yaspGraph::toggleVisibility(bool show) {
//    qDebug() << "yaspGraph::toggleVisibility: " << infoGraph->name() << " / hidden " << hidden << " / show " << show;
    if (hidden) {
        infoGraph->setVisible(false);
        refLine->setVisible(false);
    } else {
        infoGraph->setVisible(show);
        refLine->setVisible(show);
    }
}

//-----------------------------------------------------------------------------------------
void yaspGraph::setSelected(bool sel) {
//    qDebug() << "yaspGraph::setSelected: " << infoGraph->name() << " / hidden " << hidden << " / sel " << sel;
    QPen pen = infoGraph->pen();
    if (sel) {
        pen.setDashPattern(plotDashPattern);
        infoGraph->setPen(pen);
        pen.setWidth(1);
        infoText->setPen(pen);
        infoText->setSelectedPen(pen);
        infoText->setSelectedColor(pen.color());
        infoText->setColor(pen.color());
    } else {
        pen.setStyle(Qt::SolidLine);
        infoGraph->setPen(pen);
        pen.setWidth(1);
        infoText->setPen(Qt::NoPen);
        infoText->setSelectedPen(Qt::NoPen);
        infoText->setSelectedColor(pen.color());
        infoText->setColor(pen.color());
    }
    pen.setDashPattern(rLineDashPattern);
    pen.setWidthF(0.5);
    refLine->setPen(pen);
    QFont font = infoText->font();
    font.setStrikeOut(hidden);
    infoText->setFont(font);
    infoText->setSelectedFont(font);
}

//-----------------------------------------------------------------------------------------
void yaspGraph::updateLabel(QString str, int margin) {
//    lastX = lX;
    plotInfoStr = str;
    QString info = infoGraph->name() + " -> " + plotInfoStr;
    QColor color = infoGraph->pen().color();
    QFont font = infoText->font();
//    font.setPointSize(8);
////    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Bold);
//    font.setStyle(QFont::StyleItalic);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(info);
    qreal pixelsHigh = fm.height();
    QPointF labelPos;
    labelPos.setX(pixelsWide + infoText->padding().left()+ infoText->padding().right() + margin + INFO_TEXT_LEFT_MARGIN );
    labelPos.setY( (static_cast<double>(id)+1) * (pixelsHigh + infoText->padding().top() + infoText->padding().bottom()));
    infoText->setColor(color);
//    QPen penT(color);
    font.setStrikeOut(hidden);
    infoText->setFont(font);
    infoText->setSelectedFont(font);
//    infoText->setPen(penT);
    infoText->setSelectedColor(color);
    QPen pen = infoGraph->pen();
    pen.setWidth(1);
    infoText->setSelectedPen(pen);
    infoText->position->setCoords(labelPos.x(), labelPos.y());
    infoText->setText(info);
//    infoText->layer()->replot();
    QPen penL(color, 0.5);
    penL.setDashPattern(rLineDashPattern);
    refLine->setPen(penL);
    refLine->pen().setColor(color);
    refLine->point1->setCoords(0, yOffset);
    refLine->point2->setCoords(1, yOffset);

//    refLine->layer()->replot();
}

//-----------------------------------------------------------------------------------------
void yaspGraph::updateMinMax(double v) {
    if (v < minY) minY = v;
    if (v > maxY) maxY = v;
//    if (qFuzzyCompare(minY, maxY)) {
//        if (minY > yOffset) {
//            minY = yOffset;
//        }
//        if (maxY < yOffset) {
//            maxY = yOffset;
//        }
//    }
}

//-----------------------------------------------------------------------------------------
QCPItemText* yaspGraph::info() {
    return infoText;
}
////-----------------------------------------------------------------------------------------
//QCPItemStraightLine *yaspGraph::rLine() {
//    return refLine;
//}

////-----------------------------------------------------------------------------------------
//double yaspGraph::refY() {
//    return refLine->point1->coords().y();
//}

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
void yaspGraph::setOffset(double o) {
//        qDebug() << "yOffset " << yOffset << " o-> " << o;
    QSharedPointer<QCPGraphDataContainer> gData = infoGraph->data();
    for (QCPDataContainer<QCPGraphData>::iterator it = gData->begin(); it != gData->end(); ++it){
        it->value += o;
    }
    yOffset += o;
    minY += o;
    maxY += o;

    refLine->point1->setCoords(0, yOffset);
    refLine->point2->setCoords(1, yOffset);
}
//-----------------------------------------------------------------------------------------
double yaspGraph::offset() {
    return yOffset;
}
//-----------------------------------------------------------------------------------------
void yaspGraph::setMult(double m) {
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
    minY -= yOffset;
    minY /= yMult;
    minY *= nm;
    minY += yOffset;
    maxY -= yOffset;
    maxY /= yMult;
    maxY *= nm;
    maxY += yOffset;

    yMult = nm;
    refLine->point1->setCoords(0, yOffset);
    refLine->point2->setCoords(1, yOffset);
}
//-----------------------------------------------------------------------------------------
double yaspGraph::mult() {
    return yMult;
}
////-----------------------------------------------------------------------------------------
//void yaspGraph::updateRefLine(int dpn) {
//    refLine->start->setCoords(0, yOffset);
//    refLine->end->setCoords(dpn, yOffset);
//}
