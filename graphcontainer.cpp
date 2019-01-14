#include "graphcontainer.h"
#include <QDebug>
#include <QPushButton>
#include <QPalette>
#include <QSpacerItem>

graphContainer::graphContainer(QCPGraph *g, int nop, QString pName, int id, QWidget *parent)
    : QWidget(parent), graph(g), NUMBER_OF_POINTS(nop), plotName(pName) {

     //penColor = QColor::fromRgb(QRandomGenerator::global()->generate());
     QColor colours[10] = {QColor("#ffffff"), QColor("#ffff00"), QColor("#aaffaf"),
                           QColor("#ffaa00"), QColor("#ffaaff"), QColor("#00ffff"),
                           QColor("#ff0000"), QColor("#0000ff"), QColor("#00ff00"),
                           QColor("#ff00aa")};

//    // this are the numbers of the QT default colors
//    int QtColours[]= { 3,2,7,13,8,14,9, 15, 10, 16, 11, 17, 12, 18, 5, 4, 6, 19, 0, 1 };
//    int r = qrand() % ((sizeof(colours))+ 1);
//    penColor.setRgb(QtColours[r]);

//     g->setName("test");

     int c = QRandomGenerator::global()->bounded(10);
     qDebug() << "random color: " << c;
     penColor = colours[c];

     //font.setFamily("time");
     font.setPointSize(8);
     font.setWeight(50);
     font.setFixedPitch(true);
//     QString lStr =  plotName + " -> Mult = 1";
     QString lStr =  plotName + " -> Mult = 1 Delta = 0";

     textLabel = new QCPItemText(graph->parentPlot());
     textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
//     textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
//     textLabel->position->setCoords(0.05, id * 0.04); // place position at center/top of axis rect
     textLabel->position->setType(QCPItemPosition::ptAbsolute );
     textLabel->setText(lStr);
     QFontMetricsF fm(font);
     qreal pixelsWide = fm.width(lStr);
     qreal pixelsHigh = fm.height();
     labelPos.setX(pixelsWide + 100);
     labelPos.setY(10 + (id * pixelsHigh));
     textLabel->position->setCoords(labelPos.x(), labelPos.y());
     //textLabel->position->setCoords(150, 10 + (id * pixelsHigh));

     qDebug() << "plotName: " << plotName;
     textLabel->setColor(penColor);
     textLabel->setFont(font); // make font a bit larger
//     textLabel->setPen(QPen(Qt::white)); // show black border around text

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
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout = new QGridLayout();
    //layout->setContentsMargins(0,0,0,0);
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
    layout->addWidget(colorLabel, 0, 0, Qt::AlignTop);
    layout->addWidget(colorButton, 0, 1, Qt::AlignTop);
    connect(colorButton, SIGNAL (clicked()), this, SLOT (handleColor()));
    QLabel* widthLabel = new QLabel("Width");
    widthLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    widthSpinBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(widthLabel, 1 , 0, Qt::AlignTop);
    layout->addWidget(widthSpinBox, 1 , 1, Qt::AlignTop);
    connect(widthSpinBox, SIGNAL (valueChanged(int)), this, SLOT (handleWidth(int)));
//    QLabel* deltaLabel = new QLabel("Delta");
//    deltaLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//    layout->addWidget(deltaLabel, 2 , 0, Qt::AlignTop);
//    deltaSlider = new QSlider(Qt::Orientation::Horizontal);
//    layout->addWidget(deltaSlider, 2 , 1, Qt::AlignTop);
//    deltaSlider->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
//    deltaSlider->setValue(0);
//    deltaSlider->setSingleStep(10);
//    deltaSlider->setPageStep(100);
    delta = 0;
    slideDelta = new FormSliderInfo("Delta", graph->parentPlot()->yAxis->range().lower,
                                                    graph->parentPlot()->yAxis->range().upper, 0);

    slideDelta->setValue(0);
    slideDelta->setSingleStep(10);
    slideDelta->setPageStep(100);
    layout->addWidget(slideDelta, 2, 0, 1, -1, Qt::AlignTop);
    connect(slideDelta, SIGNAL (valueChanged(int)), this, SLOT (handleDelta(int)));

    mult = 1;
//    slideMult = new FormSliderInfo("Mult", 1, 1000, 1);
//    slideMult->setValue(1);
//    slideMult->setSingleStep(1);
//    slideMult->setPageStep(10);
//    layout->addWidget(slideMult, 3, 0, 1, -1, Qt::AlignTop);
//    connect(slideMult, SIGNAL (valueChanged(int)), this, SLOT (handleMult(int)));

    QLabel* comboMultLabel = new QLabel("Mult");
    QComboBox* comboMult = new QComboBox();
    comboMult->addItem("1");
    comboMult->addItem("2");
    comboMult->addItem("5");
    comboMult->addItem("10");
    comboMult->addItem("50");
    comboMult->addItem("100");
    comboMult->addItem("1000");
    comboMult->addItem("10000");
    connect(comboMult, SIGNAL (currentIndexChanged(const QString)), this, SLOT (handleComboMult(const QString)));
    layout->addWidget(comboMultLabel, 3, 0, Qt::AlignTop);
    layout->addWidget(comboMult, 3, 1, Qt::AlignTop);
}

graphContainer::~graphContainer() {
    qDebug() << "graphContainer Destructor";
}

void graphContainer::updateGraph(int pCnt) {
    NUMBER_OF_POINTS = pCnt;
    //slideDelta->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
}

void graphContainer::clearData() {
    graph->clearData();
}

void graphContainer::addData(double k, double v) {
    graph->addData(k, (v * mult) + delta);                 // Add data to Graph 0
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
    textLabel->setColor(penColor);
}

void graphContainer::handleWidth(int i) {
    pen.setWidth(i);
    graph->setPen(pen);
}

void graphContainer::handleDelta(int i) {
    qDebug() << "Delta: " << i;
    delta = i;
    QString lStr =  plotName + " -> Mult = " + QString::number(mult) + " Delta = " + QString::number(delta);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(lStr);
    textLabel->setText(lStr);
    labelPos.setX(pixelsWide + 100);
    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    graph->clearData();
}

void graphContainer::handleMult(int i) {
    qDebug() << "Mult: " << i;
    mult = i;
    graph->clearData();
}

void graphContainer::handleComboMult(const QString str) {
    qDebug() << "handleComboMult: " << str;
    mult = str.toInt();
    QString lStr =  plotName + " -> Mult = " + str + " Delta = " + QString::number(delta);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(lStr);
    textLabel->setText(lStr);
    labelPos.setX(pixelsWide + 100);
    textLabel->position->setCoords(labelPos.x(), labelPos.y());

    graph->clearData();
}
