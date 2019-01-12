#include "graphcontainer.h"
#include <QDebug>
#include <QPushButton>
#include <QPalette>

graphContainer::graphContainer(QCPGraph *g, int nop, QWidget *parent) : QWidget(parent), graph(g), NUMBER_OF_POINTS(nop) {

     //penColor = QColor::fromRgb(QRandomGenerator::global()->generate());
     QColor colours[10] = {QColor("white"), QColor("black"), QColor("cyan"),
                           QColor("red"), QColor("magenta"), QColor("green"),
                           QColor("yellow"), QColor("blue"), QColor("darkred"),
                           QColor("darkgreen")};

//    // this are the numbers of the QT default colors
//    int QtColours[]= { 3,2,7,13,8,14,9, 15, 10, 16, 11, 17, 12, 18, 5, 4, 6, 19, 0, 1 };
//    int r = qrand() % ((sizeof(colours))+ 1);
//    penColor.setRgb(QtColours[r]);
    int c = QRandomGenerator::global()->bounded(11);
    qDebug() << "random color: " << c;
    penColor = colours[c];
    pen = QPen(penColor);
    graph->setPen(pen);
    axisLine = new QCPItemLine(graph->parentPlot());
    axisLine->setPen(QPen(penColor, 1.0, Qt::DashDotLine));
    axisLine->start->setCoords(0,0);
    axisLine->end->setCoords(NUMBER_OF_POINTS,0);

    qDebug() << "lr:" << graph->parentPlot()->yAxis->range().lower
             <<" ur:" << graph->parentPlot()->yAxis->range().upper
             <<" minr:" << graph->parentPlot()->yAxis->range().minRange
             <<" maxr:" << graph->parentPlot()->yAxis->range().maxRange;
    layout = new QGridLayout();
    this->setLayout(layout);
    colorButton = new QPushButton();
    colorButton->setAutoFillBackground(true);
    colorButton->setStyleSheet("background-color:" + penColor.name() + "; color: rgb(0, 0, 0)");
    QSpinBox *widthSpinBox = new QSpinBox;
    widthSpinBox->setRange(0, 10);
    widthSpinBox->setSingleStep(1);
    widthSpinBox->setValue(1);

    QLabel* colorLabel = new QLabel("Color");
    colorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    colorButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(colorLabel, 0, 0);
    layout->addWidget(colorButton, 0, 1);
    connect(colorButton, SIGNAL (clicked()), this, SLOT (handleColor()));
    QLabel* widthLabel = new QLabel("Width");
    widthLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    widthSpinBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(widthLabel, 1 , 0);
    layout->addWidget(widthSpinBox, 1 , 1);
    connect(widthSpinBox, SIGNAL (valueChanged(int)), this, SLOT (handleWidth(int)));
    QLabel* deltaLabel = new QLabel("Delta");
    deltaLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addWidget(deltaLabel, 2 , 0);
    deltaSlider = new QSlider(Qt::Orientation::Horizontal);
    layout->addWidget(deltaSlider, 2 , 1);
    deltaSlider->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
    deltaSlider->setValue(0);
    deltaSlider->setSingleStep(10);
    deltaSlider->setPageStep(100);

    delta = 0;
    deltaSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(deltaSlider, SIGNAL (valueChanged(int)), this, SLOT (handleDelta(int)));

}

graphContainer::~graphContainer() {
    qDebug() << "graphContainer Destructor";
}

void graphContainer::updateGraph(int pCnt) {
    NUMBER_OF_POINTS = pCnt;
    deltaSlider->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
}

void graphContainer::clearData() {
    graph->clearData();
}

void graphContainer::addData(double k, double v) {
    graph->addData(k, v + delta);                 // Add data to Graph 0
    graph->removeDataBefore(k - NUMBER_OF_POINTS);           // Remove data from graph 0
    axisLine->start->setCoords(k - NUMBER_OF_POINTS, delta);
    axisLine->end->setCoords(k, delta);
}

void graphContainer::handleColor() {
    penColor = QColorDialog::getColor(penColor, this, "Select Plot Color");
    pen.setColor(penColor);
    axisLine->setPen(QPen(penColor, 1.0, Qt::DashDotLine));
    graph->setPen(pen);
    colorButton->setStyleSheet("background-color:" + penColor.name() + "; color: rgb(0, 0, 0)");
}

void graphContainer::handleWidth(int i) {
    pen.setWidth(i);
    graph->setPen(pen);
}

void graphContainer::handleDelta(int i) {
    qDebug() << "Delta: " << i;
    delta = i;
    graph->clearData();
}
