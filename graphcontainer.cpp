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
    colorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    colorButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(colorLabel, 0, 0, Qt::AlignCenter);
    layout->addWidget(colorButton, 0, 1, Qt::AlignCenter);
    connect(colorButton, SIGNAL (clicked()), this, SLOT (handleColor()));

    QPushButton* resetInfoButton = new QPushButton("Clear Info");
    layout->addWidget(resetInfoButton, 0, 2, Qt::AlignCenter);
    connect(resetInfoButton, SIGNAL (clicked()), this, SLOT (handleResetInfo()));

    radioInfo = new QRadioButton("show");
    radioInfo->setChecked(true);
    radioInfo->setAutoExclusive(false);
    layout->addWidget(radioInfo, 0, 3, Qt::AlignCenter);
    connect(radioInfo, SIGNAL(toggled(bool)), this, SLOT (handleShowPlot(bool)));

    QLabel* widthLabel = new QLabel("Width");
    widthLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    widthSpinBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(widthLabel, 1 , 0, Qt::AlignCenter);
    layout->addWidget(widthSpinBox, 1 , 1, Qt::AlignCenter);
    connect(widthSpinBox, SIGNAL (valueChanged(int)), this, SLOT (handleWidth(int)));

    logInfoBtn = new QPushButton("log Data");
    logInfoBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(logInfoBtn, 1, 2, Qt::AlignCenter);
    connect(logInfoBtn, SIGNAL(clicked()), this, SLOT (logPlotButtonClicked()));

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
    layout->addWidget(comboMultLabel, 3, 0, Qt::AlignCenter);
    layout->addWidget(comboMult, 3, 1, Qt::AlignCenter);

//    font.setFamily("Comic Sans MS");
//    font.setFamily("Time");
    font.setPointSize(7);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Medium);
    font.setStyle(QFont::StyleItalic);
    //font.setFixedPitch(true);
    dataStr =  plotName;
    textLabel = new QCPItemText(graph->parentPlot());
    textLabel->setColor(penColor);
    textLabel->setFont(font); // make font a bit larger
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
    textLabel->position->setType(QCPItemPosition::ptAbsolute );
    QFontMetricsF fm(font);
    pixelsHigh = fm.height();
//    labelPos.setX(0);
//    labelPos.setY(10 + (tabIndex * pixelsHigh));
//    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    graph->parentPlot()->clearMask();
}

/******************************************************************************************************************/
graphContainer::~graphContainer() {
//    qDebug() << "graphContainer Destructor";
    if (logFile != nullptr) {
        if(logFile->isOpen()) {
             qDebug() << "graphContainer Destructor Close Log File: " << logFile->fileName();
            logFile->close();
            delete logFile;
            logFile = nullptr;
        }
    }
}

/******************************************************************************************************************/
void graphContainer::updateGraphNop(int pCnt, bool resetDeltaValue) {
    numberOfPoints = pCnt;
    if (resetDeltaValue) {
        slideDelta->setRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
        slideDelta->setValue((graph->parentPlot()->yAxis->range().lower + graph->parentPlot()->yAxis->range().upper) / 2);
    } else {
        slideDelta->extendRange(graph->parentPlot()->yAxis->range().lower, graph->parentPlot()->yAxis->range().upper);
    }
}

/******************************************************************************************************************/
void graphContainer::updateGraphParams(QColor plotBgC) {
    plotBgColor = plotBgC;
    if (!logging) {
        textLabel->setBrush(QBrush(plotBgColor));
    }
}

/******************************************************************************************************************/
bool graphContainer::isDisplayed() {
    return radioInfo->isChecked();
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
void graphContainer::setRadioInfo(bool checked) {
    radioInfo->setChecked(checked);
}

/******************************************************************************************************************/
void graphContainer::handleShowPlot(bool state) {
    graph->setVisible(state);
    updateLabel();
    graph->parentPlot()->replot();
}

/******************************************************************************************************************/
void graphContainer::setColor(QColor color) {
//    qDebug() << "graphContainer setColor " << tabPos << " color:" << color;
    penColor = color;
    pen.setColor(penColor);
    axisLine->setPen(QPen(penColor, 1.0, Qt::DashDotLine));
    graph->setPen(pen);
    colorButton->setStyleSheet("background-color:" + penColor.name() + "; color: rgb(0, 0, 0)");
    textLabel->setColor(penColor);
    emit plotColorChanged(tabPos, penColor);
}

/******************************************************************************************************************/
void graphContainer::addData(double k, double v, int time) {
    if (radioInfo->isChecked()) {
        dataMin = qMin(dataMin, v);
        dataMax = qMax(dataMax, v);
    //    dataAverage = v;
        int avr = 5;
        dataAverage = ((dataAverage * avr) + v) / (avr + 1);
        double plotV = (v * mult) + delta;
        graph->addData(k, plotV);                 // Add data to Graph 0
        graph->removeDataBefore(k - numberOfPoints);           // Remove data from graph 0
        axisLine->start->setCoords(k - numberOfPoints, delta);
        axisLine->end->setCoords(k, delta);
        if (logFile != nullptr) {
            if (isDisplayed()) {
                streamLog << k << ";" << time << ";" << v << ";" << dataAverage << ";" << plotV
                          << ";" << mult << ";" << delta << ";"
                          << "\n";
            }
        }
    }
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::logPlotButtonClicked() {
    qDebug() << "logPlotButtonClicked ";
    if (logFile == nullptr) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Log Plot"),
                                   plotName,
                                   tr("Data (*.csv)"));
        qDebug() << "Log Plot : " << fileName;
        logFile = new QFile(fileName);
        if (!logFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     logFile->errorString());
            logFile = nullptr;
            return;
        }
        qDebug() << "Log Plot Opened : " << logFile->fileName();
        streamLog.setDevice(logFile);
        QLocale locale = QLocale("fr_FR");
        locale.setNumberOptions(QLocale::OmitGroupSeparator);
        //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        streamLog.setLocale(locale);
        // Set Headers
        streamLog << "POINT" << ";" << "TIME" << ";" << "VALUE" << ";" << "dataAverage" << ";" << "plotV"
                  << ";" << "mult" << ";" << "delta"
                  << "\n";
        logging = true;
        textLabel->setBrush(QBrush(Qt::gray));
        logInfoBtn->setText("Stop Logging");
    } else {
        qDebug() << "Log Plot Closed : " << logFile->fileName();
        logInfoBtn->setText("log Data");
        logFile->close();
        delete logFile;
        logFile = nullptr;
        textLabel->setBrush(QBrush(plotBgColor));
        logging = false;
    }
}

/******************************************************************************************************************/
void graphContainer::updateLabel() {
    dataStr = "";
    if (radioInfo->isChecked()) {
        dataStr =  plotName + " -> Mult = " + QString::number(mult) + " Delta = " + QString::number(delta)
                + " Min = " + QString::number(dataMin)
                + " Max = " + QString::number(dataMax)
                + " Val = " + QString::number(dataAverage );

    }
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(dataStr);
//    qreal pixelsHigh = fm.height();
    labelPos.setX(pixelsWide + 100);
    labelPos.setY(10 + (tabPos * pixelsHigh));
    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    textLabel->setText(dataStr);
}

/******************************************************************************************************************/
void graphContainer::handleColor() {
    penColor = QColorDialog::getColor(penColor, this, "Select Plot Color");
    setColor(penColor);
    updateLabel();
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
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::handleDelta(int i) {
//    qDebug() << "Delta: " << i;
    delta = i;
    clearData();
    updateLabel();
}

/******************************************************************************************************************/
void graphContainer::handleComboMult(const QString str) {
    qDebug() << "handleComboMult: " << str;
    mult = str.toInt();
    clearData();
    updateLabel();
}
