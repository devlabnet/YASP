#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

#define INFO_TEXT_LEFT_MARGIN 50

class yaspGraph {
public:
    yaspGraph(int id, QCPGraph* g, QCPItemText* info, QCPItemStraightLine* rLine, QString plotStr, QColor color, double plotTimeInSeconds);
    QCPItemText* info();
//    QCPItemStraightLine* rLine();
    QCPGraph* plot();
    void addData(double v);
    void save(QTextStream& streamData);
    void setOffset(double o);
    double offset();
    void setMult(double m);
    double mult();
//    double refY();
    QString infoStr() {
        return plotInfoStr;
    }
    void updateLabel(QString str, int margin);
//    void updateRefLine(int dpn);
    void setSelected(bool sel);
    void reset();
    void toggleVisibility(bool show);
    void hide(bool h);
    double getMin() { return minY;}
//    void setMin(double m) { minY = m;}
    double getMax() { return maxY; }
//    void setMax(double m) { maxY = m;}
private:
    int id;
    QVector<qreal> plotDashPattern;
    QVector<qreal> rLineDashPattern;
    QCPGraph* infoGraph;
    QCPItemText* infoText;
    QCPItemStraightLine* refLine;
//    double lastX;
    double yOffset;
    double yMult;
    double minY;
    double maxY;
    QString plotInfoStr;
    bool hidden;
};

#endif // YASPGRAPH_H
