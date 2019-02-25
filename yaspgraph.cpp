#include "yaspgraph.h"

yaspGraph::yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine, QString plotStr,
                     QColor color, double plotTimeInSeconds)
    : id(id), infoGraph(g), infoText(info), refLine(rLine) {
    yOffset = 0;
    yMult = 1.0;
    plotDashPattern = QVector<qreal>() << 16 << 4 << 8 << 4;
    rLineDashPattern = QVector<qreal>() << 64 << 4 ;

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
    rLine->start->setCoords(0,0);
    rLine->end->setCoords(plotTimeInSeconds, 0);
    tracerVisibleDef = true;
   // textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText());
//    dataContainer = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
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
    yOffset = 0;
    yMult = 1.0;
}

//-----------------------------------------------------------------------------------------
void yaspGraph::toggleTracerVisibility(bool show) {
    if (show) {
        infoGraph->setVisible(tracerVisibleDef);
    } else {
        tracerVisibleDef = infoGraph->visible();
        infoGraph->setVisible(false);
    }
}

//-----------------------------------------------------------------------------------------
void yaspGraph::setSelected(bool sel) {
//     qDebug() << "yaspGraph::setSelected: " << sel;
    QPen pen = infoGraph->pen();
    pen.setWidth(1);
    if (sel) {
        pen.setDashPattern(plotDashPattern);
        infoGraph->setPen(pen);
        infoText->setPen(pen);
        infoText->setSelectedPen(pen);
        infoText->setSelectedColor(pen.color());
        infoText->setColor(pen.color());
    } else {
        pen.setStyle(Qt::SolidLine);
        infoGraph->setPen(pen);
        infoText->setPen(Qt::NoPen);
        infoText->setSelectedPen(Qt::NoPen);
        infoText->setSelectedColor(pen.color());
        infoText->setColor(pen.color());
    }
    pen.setDashPattern(rLineDashPattern);
    pen.setWidthF(0.5);
    refLine->setPen(pen);
}

//-----------------------------------------------------------------------------------------
void yaspGraph::updateLabel(QString str, double lX, int margin) {
    lastX = lX;
    plotInfoStr = str;
    QString info = infoGraph->name() + " -> " + plotInfoStr;
    QColor color = infoGraph->pen().color();
    QFont font;
    font.setPointSize(7);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Medium);
    font.setStyle(QFont::StyleItalic);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(info);
    qreal pixelsHigh = fm.height();
    QPoint labelPos;
    labelPos.setX(pixelsWide + infoText->padding().left()+ infoText->padding().right() + margin + 70 );
    labelPos.setY( (id+1) * (5 + (pixelsHigh + infoText->padding().top() + infoText->padding().bottom())));
    infoText->setColor(color);
//    infoText->setPen(infoGraph->pen());
    infoText->setSelectedColor(color);
    infoText->setSelectedPen(infoGraph->pen());
    infoText->position->setCoords(labelPos.x(), labelPos.y());
    infoText->setText(info);
//    infoText->layer()->replot();
    QPen pen(color, 0.5);
    pen.setDashPattern(rLineDashPattern);
    refLine->setPen(pen);
    refLine->pen().setColor(color);
    refLine->start->setCoords(0, yOffset);
    refLine->end->setCoords(lastX, yOffset);

//    refLine->layer()->replot();
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
void yaspGraph::setOffset(double o) {
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
    refLine->end->setCoords(lastX, yOffset);
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
    yMult = nm;
    refLine->start->setCoords(0, yOffset);
    refLine->end->setCoords(lastX, yOffset);
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
