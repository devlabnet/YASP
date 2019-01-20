#ifndef GRAPHCONTAINER_H
#define GRAPHCONTAINER_H

#include <QWidget>
#include "qcustomplot.h"
#include <QVBoxLayout>
#include "formsliderinfo.h"

class graphContainer : public QWidget
{
    Q_OBJECT
public:
    explicit graphContainer(QCPGraph* g, int nop, QString pName, QColor color, int id, QWidget *parent = nullptr);
    ~graphContainer();

    QCPGraph* getGraph() { return graph; }
    void addData(double k, double v);
    void clearData();
    void clearLabels();
    void updateGraph(int pCnt,  bool resetDeltaValue = false);
    QString getName() { return plotName; }
    void setColor(QColor color);
    void setName(QString name) { plotName = "Plot " + name; }
    QString getDataStr() { return dataStr; }
    bool isUsed() { return inUse; }
    void setUsed(bool b) { inUse = b; }
    int getTabPos() { return tabPos; }
    void setTabPos(int p) { tabPos = p; }
    void setRadioInfo(bool checked);

private:
    QCPGraph* graph;
    // Number of points plotted
    int numberOfPoints;
    bool inUse = false;
    int tabPos = -1;
    QString plotName;
    QString dataStr;
    int tabIndex;
    QFont font;
    QCPItemText *textLabel;
    qreal pixelsHigh;
    QPoint labelPos;
    int viewIndex = 0;
    QGridLayout* layout;
    QPushButton *colorButton;
    QRadioButton* radioInfo;
    QPen pen;
    QColor penColor;
    QCPItemLine *axisLine;
    int delta;
    int mult;
    FormSliderInfo* slideDelta;
    FormSliderInfo* slideMult;
    double dataMin = 0;
    double dataMax = 0;
    double dataAverage = 0;
    void updateLabel();

private slots:
   void handleColor();
   void handleWidth(int i);
   void handleDelta(int i);
   void handleResetInfo();
//   void handleMult(int i);
    void handleComboMult(const QString str);
    void handleShowPlot(bool state);

signals:
    void plotColorChanged(int, QColor);
public slots:

};

#endif // GRAPHCONTAINER_H
