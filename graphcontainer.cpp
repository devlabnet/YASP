#include "graphcontainer.h"
#include <QDebug>
#include <QPushButton>
#include <QPalette>
#include <QSpacerItem>

/******************************************************************************************************************/
graphContainer::graphContainer(QCPGraph *g, int nop, QString pName, QColor color, int id, QWidget *parent)
    : QWidget(parent), graph(g), numberOfPoints(nop), plotName(pName), tabIndex(id), penColor(color) {

    pen = QPen(penColor);
    graph->setPen(pen);
    axisLine = new QCPItemLine(graph->parentPlot());
    axisLine->setPen(QPen(penColor, 1.0, Qt::DashDotLine));
    axisLine->start->setCoords(0,0);
    axisLine->end->setCoords(numberOfPoints,0);
//    qDebug() << "lr:" << graph->parentPlot()->yAxis->range().lower
//             <<" ur:" << graph->parentPlot()->yAxis->range().upper
//             <<" minr:" << graph->parentPlot()->yAxis->range().minRange
//             <<" maxr:" << graph->parentPlot()->yAxis->range().maxRange;
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout = new QGridLayout();
    //layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);
    colorButton = new QPushButton();
    colorButton->setAutoFillBackground(true);
    colorButton->setStyleSheet("background-color:" + penColor.name() + "; color: rgb(0, 0, 0); border: 2px solid black");
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

    QPushButton* resetInfoButton = new QPushButton("Clear Info");
    layout->addWidget(resetInfoButton, 0, 2, Qt::AlignTop);
    connect(resetInfoButton, SIGNAL (clicked()), this, SLOT (handleResetInfo()));

    chkBox = new QCheckBox("show");
    chkBox->setChecked(true);
    layout->addWidget(chkBox, 0, 3, Qt::AlignTop);
    connect(chkBox, SIGNAL(stateChanged(int)), this, SLOT (handleShowPlot(int)));

    QLabel* widthLabel = new QLabel("Width");
    widthLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    widthSpinBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(widthLabel, 1 , 0, Qt::AlignTop);
    layout->addWidget(widthSpinBox, 1 , 1, Qt::AlignTop);
    connect(widthSpinBox, SIGNAL (valueChanged(int)), this, SLOT (handleWidth(int)));
    delta = 0;
    slideDelta = new FormSliderInfo("Delta",
                                    graph->parentPlot()->yAxis->range().lower,
                                    graph->parentPlot()->yAxis->range().upper, 0);
    slideDelta->setValue(0);
    slideDelta->setSingleStep(10);
    slideDelta->setPageStep(100);
    layout->addWidget(slideDelta, 2, 0, 1, -1, Qt::AlignTop);
    connect(slideDelta, SIGNAL (valueChanged(int)), this, SLOT (handleDelta(int)));

    mult = 1;
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

    //font.setFamily("time");
    font.setPointSize(8);
    font.setWeight(50);
    font.setFixedPitch(true);
    dataStr =  plotName;
    textLabel = new QCPItemText(graph->parentPlot());
    textLabel->setColor(penColor);
    textLabel->setFont(font); // make font a bit larger
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
     textLabel->position->setType(QCPItemPosition::ptAbsolute );
    QFontMetricsF fm(font);
    qreal pixelsHigh = fm.height();
    labelPos.setX(0);
    labelPos.setY(10 + (tabIndex * pixelsHigh));
    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    graph->parentPlot()->clearMask();
}

/******************************************************************************************************************/
graphContainer::~graphContainer() {
//    qDebug() << "graphContainer Destructor";
}

/******************************************************************************************************************/
void graphContainer::updateGraph(int pCnt, bool resetDeltaValue) {
    numberOfPoints = pCnt;
    if (resetDeltaValue) {
        slideDelta->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
        slideDelta->setValue((graph->parentPlot()->yAxis->range().lower + graph->parentPlot()->yAxis->range().upper) / 2);
    } else {
        slideDelta->extendRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
    }
}

/******************************************************************************************************************/
void graphContainer::clearData() {
    graph->clearData();
    handleResetInfo();
}

/******************************************************************************************************************/
void graphContainer::clearLabels() {
    delete textLabel;
}

/******************************************************************************************************************/
void graphContainer::handleShowPlot(int state) {
    if (state == Qt::Unchecked) {
        graph->setVisible(false);
    } else {
        graph->setVisible(true);
    }
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::setColor(QColor color) {
    qDebug() << "graphContainer setColor " << tabIndex << " color:" << color;
    penColor = color;
    pen.setColor(penColor);
    axisLine->setPen(QPen(penColor, 1.0, Qt::DashDotLine));
    graph->setPen(pen);
    colorButton->setStyleSheet("background-color:" + penColor.name() + "; color: rgb(0, 0, 0)");
    textLabel->setColor(penColor);
    emit plotColorChanged(tabIndex, penColor);
}

/******************************************************************************************************************/
void graphContainer::addData(double k, double v) {
    if (tabIndex == 8) {
        qDebug() << "--> " << k << " / " << v;
    }
    if (chkBox->isChecked()) {
        dataMin = qMin(dataMin, v);
        dataMax = qMax(dataMax, v);
    //    dataAverage = v;
        int avr = 5;
        dataAverage = ((dataAverage * avr) + v) / (avr + 1);
        graph->addData(k, (v * mult) + delta);                 // Add data to Graph 0
        graph->removeDataBefore(k - numberOfPoints);           // Remove data from graph 0
        axisLine->start->setCoords(k - numberOfPoints, delta);
        axisLine->end->setCoords(k, delta);
    }
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::updateLabel() {
    dataStr = "";
    if (chkBox->isChecked()) {
        dataStr =  plotName + " -> Mult = " + QString::number(mult) + " Delta = " + QString::number(delta)
                + " Min = " + QString::number(dataMin)
                + " Max = " + QString::number(dataMax)
                + " Val = " + QString::number(dataAverage );

    }
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(dataStr);
//    qreal pixelsHigh = fm.height();
    labelPos.setX(pixelsWide + 100);
    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    textLabel->setText(dataStr);
}

/******************************************************************************************************************/
void graphContainer::handleColor() {
    penColor = QColorDialog::getColor(penColor, this, "Select Plot Color");
    setColor(penColor);
}

/******************************************************************************************************************/
void graphContainer::handleResetInfo() {
    dataMin = 0;
    dataMax = 0;
    dataAverage = 0;
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::handleWidth(int i) {
    pen.setWidth(i);
    graph->setPen(pen);
}

/******************************************************************************************************************/
void graphContainer::handleDelta(int i) {
//    qDebug() << "Delta: " << i;
    delta = i;
    clearData();
}

/******************************************************************************************************************/
void graphContainer::handleComboMult(const QString str) {
    qDebug() << "handleComboMult: " << str;
    mult = str.toInt();
    updateLabel();
    clearData();
}
