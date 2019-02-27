#ifndef YASPGRAPH_H
#define YASPGRAPH_H
#include <qcustomplot.h>

#define INFO_TEXT_LEFT_MARGIN 50

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
    QString infoStr() {
        return plotInfoStr;
    }
    void updateLabel(QString str, double lX, int margin);
//    void updateRefLine(int dpn);
    void setSelected(bool sel);
    void reset();
    void toggleVisibility(bool show);
    void hide(bool h);
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
    QString plotInfoStr;
    bool hidden;
};

#endif // YASPGRAPH_H
