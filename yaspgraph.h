#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

class yaspGraph {
public:
    yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemLine* rLine, QString plotStr, QColor color, double plotTimeInSeconds);
    QCPItemText* info();
    QCPItemLine* rLine();
    QCPGraph* plot();
    void save(QTextStream& streamData);
    void setOffset(double o);
    double offset();
    void setMult(double m);
    double mult();
    void updateLabel(QString info, double lX, int margin);
//    void updateRefLine(int dpn);
    void setSelected(bool sel);
    void reset();
private:
    int id;
    QVector<qreal> plotDashPattern;
    QVector<qreal> rLineDashPattern;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemLine* refLine;
    double lastX;
    double yOffset;
    double yMult;
};

#endif // YASPGRAPH_H
