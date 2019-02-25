/***************************************************************************
**  This file is part of Serial Port Plotter                              **
**                                                                        **
**                                                                        **
**  Serial Port Plotter is a program for plotting integer data from       **
**  serial port using Qt and QCustomPlot                                  **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Borislav                                             **
**           Contact: b.kereziev@gmail.com                                **
**           Date: 29.12.14                                               **
****************************************************************************/

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QtGui>
#include <QPen>

/******************************************************************************************************************/
/* Constructor */
/******************************************************************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected(false), plotting(false), dataPointNumber(0), numberOfAxes(1),
    STATE(WAIT_START), plotTimeInSeconds(PLOT_TIME_DEF) {
    ui->setupUi(this);
    QLocale::setDefault(QLocale::C);
    createUI();      // Create the UI
    QColor gridColor = QColor(170,170,170);
    ui->bgColorButton->setAutoFillBackground(true);
    ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");
    QColor subGridColor = QColor(80,80,80);
    ui->plot->setBackground(QBrush(bgColor));                                    // Background for the plot area
    ui->plot->hide();
    ui->stopPlotButton->setEnabled(false);                                                // Plot button is disabled initially
    ui->logPlotButton->setVisible(false);
    // Legend
    ui->plot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->plot->setNotAntialiasedElements(QCP::aeAll);                                      // used for higher performance (see QCustomPlot real time example)
    // Y Axes
    ui->plot->yAxis->setTickLabelColor(gridColor);                              // See QCustomPlot examples / styled demo
    ui->plot->xAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridPen(QPen(subGridColor, 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setSubGridPen(QPen(subGridColor, 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridVisible(true);
    ui->plot->yAxis->grid()->setSubGridVisible(true);
    ui->plot->yAxis->setBasePen(QPen(gridColor));
    ui->plot->yAxis->setTickPen(QPen(gridColor));
    ui->plot->yAxis->setSubTickPen(QPen(gridColor));
    ui->plot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    // X Axes
    ui->plot->xAxis->setBasePen(QPen(gridColor));
    ui->plot->xAxis->setSubTickPen(QPen(gridColor));
    ui->plot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    ui->plot->xAxis->setTickLabelColor(gridColor);
    fixedTicker = QSharedPointer<QCPAxisTickerFixed>(new QCPAxisTickerFixed);
    plotTimeInSeconds = 30;
    ui->plot->xAxis->setTicker(fixedTicker);
    // tick step shall be 0.001 second -> 1 milisecond
    // tick step shall be 0.0001 second -> 0.1 milisecond -> 100 microdeconds
    // tick step shall be 0.00001 second -> 0.01 milisecond -> 10 microdeconds
    fixedTicker->setTickStep(0.00001);
    fixedTicker->setTickCount(10);
    fixedTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
//    fixedTicker->setTickStepStrategy( QCPAxisTicker::tssReadability);
    fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples );
    ui->plot->xAxis->setTickPen(QPen(Qt::red, 2));
    ui->plot->xAxis->setTickLength(15);
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(xAxisRangeChanged(const QCPRange&)));

    // Slot for printing coordinates
    connect(ui->plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePressInPlot(QMouseEvent*)));
//    connect(ui->plot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(onMouseDoubleClickInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMoveInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseReleaseInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onMouseWheelInPlot(QWheelEvent*)));
//    connect(ui->plot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedByUserInPlot()));
    //    // setup policy and connect slot for context menu popup:
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(ui->plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(plotContextMenuRequest(QPoint)));
    serialPort = nullptr;
    // Connect update timer to replot slot
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(replot()));
    ui->menuWidgets->menuAction()->setVisible(false);
    updateTimer.setInterval(20);
    QPalette p;
    p.setColor(QPalette::Background, QColor(144, 238, 144));
    ui->splitter->setPalette(p);
    ui->tabWidget->setCurrentIndex(0);
    ui->spinDisplayTime->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->spinDisplayTime->setMinimum(PLOT_TIME_MIN_DEF);
    ui->spinDisplayTime->setMaximum(PLOT_TIME_MAX_DEF);
    ui->spinDisplayTime->setSingleStep(PLOT_TIME_STEP_DEF);
    ui->spinDisplayTime->setValue(PLOT_TIME_DEF);
    ui->spinDisplayTime->setDecimals(1);
    ui->spinDisplayTime->setSuffix(" Secs");
    ui->autoScrollLabel->setStyleSheet("QLabel { color : DodgerBlue; }");
    ui->autoScrollLabel->setText("Auto Scroll OFF, To allow move cursor to the end or SELECT Button ---> ");
    ui->tabWidget->removeTab(1);

//    groupBoxPlotSelectionHbox = new QHBoxLayout;
//    ui->groupBoxPlotSelection->setLayout(groupBoxPlotSelectionHbox);
//    selectButtonsGroup = new QButtonGroup();
//    selectButtonsGroup->setExclusive(true);

    contextMenu = new QMenu(this);
    connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuTriggered(QAction*)));
    connect(contextMenu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    // Clear the terminal
    on_clearTermButton_clicked();
//    ticksXTime.start();
}

/******************************************************************************************************************/
/* Destructor */
/******************************************************************************************************************/
MainWindow::~MainWindow() {
    if(serialPort != nullptr) delete serialPort;
    delete ui;
}
/******************************************************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event) {
   if (logFile != nullptr) {
       if(logFile->isOpen()) {
           logFile->close();
           delete logFile;
           logFile = nullptr;
       }
   }
   // or event->accept(); but fine 'moments' are there
   QMainWindow::closeEvent(event);
}


/******************************************************************************************************************/
/**Create the GUI */
/******************************************************************************************************************/
void MainWindow::createUI() {
    if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
        enableControls(false);
        ui->connectButton->setEnabled(false);
        ui->statusBar->setStyleSheet("color: Black; background-color: Tomato;");
        ui->statusBar->showMessage("No ports detected.");
        ui->saveJPGButton->setEnabled(false);
        return;
    }
    for(QSerialPortInfo port : QSerialPortInfo::availablePorts()) {                       // List all available serial ports and populate ports combo box
        ui->comboPort->addItem(port.portName());
    }
    ui->comboBaud->addItem("1200");                                                       // Populate baud rate combo box
    ui->comboBaud->addItem("2400");
    ui->comboBaud->addItem("4800");
    ui->comboBaud->addItem("9600");
    ui->comboBaud->addItem("19200");
    ui->comboBaud->addItem("38400");
    ui->comboBaud->addItem("57600");
    ui->comboBaud->addItem("115200");
    ui->comboBaud->setCurrentIndex(7);                                                    // Select 9600 bits by default
    ui->comboData->addItem("8 bits");                                                     // Populate data bits combo box
    ui->comboData->addItem("7 bits");
    ui->comboParity->addItem("none");                                                     // Populate parity combo box
    ui->comboParity->addItem("odd");
    ui->comboParity->addItem("even");
    ui->comboStop->addItem("1 bit");                                                      // Populate stop bits combo box
    ui->comboStop->addItem("2 bits");
}

/******************************************************************************************************************/
/* Enable/disable controls */
/******************************************************************************************************************/
void MainWindow::enableControls(bool enable) {
    ui->comboBaud->setEnabled(enable);                                                    // Disable controls in the GUI
    ui->comboData->setEnabled(enable);
    ui->comboParity->setEnabled(enable);
    ui->comboPort->setEnabled(enable);
    ui->comboStop->setEnabled(enable);
}

/******************************************************************************************************************/
void MainWindow::cleanGraphs() {
    ui->plot->clearItems();
    ui->plot->clearGraphs();
    ui->plot->hide();
    graphs.clear();
    infoModeLabel = nullptr;
}

/******************************************************************************************************************/
void MainWindow::cleanGraphsBefore(double d) {
    d -= PLOT_TIME_MAX_DEF/2;
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->plot()->data()->removeBefore(d);
    }
    cleanDataTtime += PLOT_TIME_MAX_DEF;
}

/******************************************************************************************************************/
yaspGraph* MainWindow::addGraph(int id) {
        ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
//        ui->plot->xAxis->setRange(0, plotTimeInSeconds);                                      // Set x axis range for specified number of points
        QString plotStr = "Plot " + QString::number(id);
        QCPGraph* graph = ui->plot->addGraph();
//        plotDashPattern = QVector<qreal>() << 16 << 4 << 8 << 4;
//        graph->setName(plotStr);
//        graph->setPen(QPen(colours[id]));
        QCPItemText* textLabel = new QCPItemText(ui->plot);
        textLabel->setRoundCorners(5);
        textLabel->setPadding(QMargins(8, 4, 8, 4));

        textLabel->setProperty("id", id);
//        textLabel->setColor(colours[id]);
//        textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
//        textLabel->position->setType(QCPItemPosition::ptAbsolute );
//        textLabel->setSelectable(true);
//        textLabel->setPadding(QMargins(2,2,2,2));
        connect(textLabel, SIGNAL(selectionChanged (bool)), this, SLOT(plotLabelSelected(bool)));
        QCPItemLine* axisLine = new QCPItemLine(ui->plot);
//        QPen pen = QPen(colours[id], 0.5);
//        rLineDashPattern = QVector<qreal>() << 64 << 4 ;
//        pen.setDashPattern(rLineDashPattern);
//        axisLine->setPen(pen);
//        axisLine->start->setCoords(0,0);
//        axisLine->end->setCoords(plotTimeInSeconds, 0);
        yaspGraph* g = new yaspGraph(id, graph, textLabel, axisLine, plotStr, colours[id], plotTimeInSeconds);
        graphs.insert(id, g);
        return g;
}

/******************************************************************************************************************/
void MainWindow::updateLabel(int id, QString plotInfoStr) {
    yaspGraph* yGraph = graphs[id];
    Q_ASSERT(yGraph);
    yGraph->updateLabel(plotInfoStr, lastDataTtime, ui->plot->yAxis->axisRect()->left());
    if (selectedPlotId == id) {
        infoModeLabel->setColor(yGraph->plot()->pen().color());
    }
}

/******************************************************************************************************************/
/* Open the inside serial port; connect its signals */
/******************************************************************************************************************/
void MainWindow::openPort() {
    // Clear the terminal
    on_clearTermButton_clicked();
    // Get parameters from controls first
    QSerialPortInfo portInfo(ui->comboPort->currentText());                           // Temporary object, needed to create QSerialPort
    int baudRate = ui->comboBaud->currentText().toInt();                              // Get baud rate from combo box
    int dataBitsIndex = ui->comboData->currentIndex();                                // Get index of data bits combo box
    int parityIndex = ui->comboParity->currentIndex();                                // Get index of parity combo box
    int stopBitsIndex = ui->comboStop->currentIndex();                                // Get index of stop bits combo box
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    if(dataBitsIndex == 0) {                                                          // Set data bits according to the selected index
        dataBits = QSerialPort::Data8;
    } else {
        dataBits = QSerialPort::Data7;
    }
    if(parityIndex == 0) {                                                            // Set parity according to the selected index
        parity = QSerialPort::NoParity;
    } else if(parityIndex == 1) {
        parity = QSerialPort::OddParity;
    } else {
        parity = QSerialPort::EvenParity;
    }
    if(stopBitsIndex == 0) {                                                          // Set stop bits according to the selected index
        stopBits = QSerialPort::OneStop;
    } else {
        stopBits = QSerialPort::TwoStop;
    }
    serialPort = new QSerialPort(portInfo);                                        // Use local instance of QSerialPort; does not crash
    serialPort->setBaudRate(baudRate, QSerialPort::AllDirections);
    serialPort->setParity(parity);
    serialPort->setDataBits(dataBits);
    serialPort->setStopBits(stopBits);
    if (serialPort->open(QIODevice::ReadWrite) ) {
        ui->plot->show();
        receivedData.clear();
        noMsgReceivedData.clear();
        portOpenedSuccess();
    } else {
        ui->statusBar->setStyleSheet("background-color: Tomato;");
        ui->statusBar->showMessage("Cannot open port " + ui->comboPort->currentText() + " --> " + serialPort->errorString());
        qDebug() << "Cannot open port " << ui->comboPort->currentText();
        qDebug() << serialPort->errorString();
    }
}

/******************************************************************************************************************/
/* Port Combo Box index changed slot; displays info for selected port when combo box is changed */
/******************************************************************************************************************/
void MainWindow::on_comboPort_currentIndexChanged(const QString &arg1) {
    QSerialPortInfo selectedPort(arg1);                                                   // Dislplay info for selected port
    ui->statusBar->setStyleSheet("background-color: SkyBlue ;");
    ui->statusBar->showMessage(selectedPort.description());
}

/******************************************************************************************************************/
/* Connect Button clicked slot; handles connect and disconnect */
/******************************************************************************************************************/
void MainWindow::on_connectButton_clicked() {
    if (connected) {
        closePort();
    } else {                                                                              // If application is not connected, connect
        openPort();                         // Open serial port and connect its signals
    }
}

/******************************************************************************************************************/
/* Slot for port opened successfully */
/******************************************************************************************************************/
void MainWindow::portOpenedSuccess() {

    connect(serialPort, SIGNAL(dataTerminalReadyChanged(bool)), this, SLOT(dataTerminalReadyChanged(bool)));
    serialPort->setDataTerminalReady(false);
    ui->menuWidgets->menuAction()->setVisible(true);
    if ((widgets != nullptr)) {
        widgets->setSerialPort(serialPort);
    }
    ui->connectButton->setText("Disconnect");                                             // Change buttons
    ui->statusBar->setStyleSheet("background-color: SpringGreen ;");
    ui->statusBar->showMessage("Connected!");
    enableControls(false);                                                                // Disable controls if port is open
    ui->stopPlotButton->setText("Stop Plot");                                             // Enable button for stopping plot
    ui->stopPlotButton->setEnabled(true);
    ui->saveJPGButton->setEnabled(true);                                                  // Enable button for saving plot
    // Slot is refreshed 20 times per second
    connected = true;                                                                     // Set flags
    plotting = true;
    ui->tabWidget->setCurrentIndex(1);
    // Reset the Device via DTR
    serialPort->setDataTerminalReady(true);
    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    connect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    QWidget* tabW = ui->tabWidget->findChild<QWidget *>("tabPlots");
    qDebug() << "tabW : " << tabW;
    ui->tabWidget->insertTab(1, tabW, "Plots");
    ui->tabWidget->setCurrentIndex(1);

    infoModeLabel = new QCPItemText(ui->plot);
    infoModeLabel->setRoundCorners(15);
    infoModeLabel->setBrush(QBrush(QColor(105,105,105)));
    infoModeLabel->setPadding(QMargins(8, 8, 8, 8));

    infoModeLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
    infoModeLabel->position->setType(QCPItemPosition::ptAbsolute );
//    infoModeLabel->setPadding(QMargins(2,2,2,2));
    QFont font;
    font.setPointSize(12);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Bold);
    font.setStyle(QFont::StyleItalic);
    infoModeLabel->setFont(font);
    QFontMetricsF fm(font);
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    qDebug() << "pixelsWide " << pixelsWide;
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 64, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
//    qDebug() << "ui->plot->geometry: " << ui->plot->geometry();
    updateTimer.start();
    tracer = new QCPItemTracer(ui->plot);
    tracer->setVisible(false);
    tracer->blockSignals(true);
    tracer->setSize(2);
    QColor linesColor;
    if (bgColor.lightness() > 128) {
        linesColor = Qt::black;
    } else {
        linesColor = Qt::white;
    }
    QPen pen = QPen(linesColor, 2);
    tracer->setPen(pen);
    tracer->setSelectable(false);

    tracerRect =  new QCPItemRect(ui->plot);
    tracerRect->setRoundCorners(25);
    tracerRect->setVisible(false);
    tracerRect->setPen(pen);
    tracerRect->setBrush(QBrush(QColor(200, 200, 200, 25)));
    pen.setStyle(Qt::DotLine);
    traceLineBottom = new QCPItemLine(ui->plot);
    pen.setColor(Qt::green);
    traceLineBottom->setPen(pen);
    traceLineTop = new QCPItemLine(ui->plot);
    pen.setColor(Qt::red);
    traceLineTop->setPen(pen);
//    traceLineBottom->setVisible(false);
//    traceLineBottom->start->setCoords(0, DBL_MAX);
//    traceLineBottom->end->setCoords(DBL_MAX, DBL_MAX);
    font.setPixelSize(12);
    tracerArrowAmplitude = new QCPItemLine(ui->plot);
    tracerArrowAmplitude->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitude->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitude->setPen(QPen(QColor(150, 255, 255), 4));
    tracerArrowAmplitudeTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeTxt->setRoundCorners(5);

    tracerArrowAmplitudeTxt->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
    tracerArrowAmplitudeTxt->setTextAlignment(Qt::AlignLeft);
    tracerArrowAmplitudeTxt->brush().setStyle(Qt::SolidPattern);
    tracerArrowAmplitudeTxt->setFont(font);
    tracerArrowAmplitudeTxt->setPen(QPen(bgColor));
    tracerArrowAmplitudeTxt->setColor(bgColor);
    tracerArrowAmplitudeTxt->setBrush(QBrush(QColor(200, 255, 255)));
    tracerArrowAmplitudeTxt->setPadding(QMargins(8, 4, 8, 4));

    tracerArrowAmplitudeTop = new QCPItemLine(ui->plot);
    tracerArrowAmplitudeTop->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeTop->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeTop->setPen(QPen(QColor(255, 150, 150), 4));
    tracerArrowAmplitudeTopTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeTopTxt->setRoundCorners(5);
    tracerArrowAmplitudeTopTxt->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    tracerArrowAmplitudeTopTxt->setTextAlignment(Qt::AlignRight);
    tracerArrowAmplitudeTopTxt->brush().setStyle(Qt::SolidPattern);
    tracerArrowAmplitudeTopTxt->setFont(font);
    tracerArrowAmplitudeTopTxt->setPen(QPen(bgColor));
    tracerArrowAmplitudeTopTxt->setColor(bgColor);
    tracerArrowAmplitudeTopTxt->setBrush(QBrush(QColor(255, 150, 150)));
    tracerArrowAmplitudeTopTxt->setPadding(QMargins(8, 4, 8, 4));

    tracerArrowAmplitudeBottom = new QCPItemLine(ui->plot);
    tracerArrowAmplitudeBottom->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeBottom->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeBottom->setPen(QPen(QColor(150, 255, 150), 4));
    tracerArrowAmplitudeBottomTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeBottomTxt->setRoundCorners(5);
    tracerArrowAmplitudeBottomTxt->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    tracerArrowAmplitudeBottomTxt->setTextAlignment(Qt::AlignRight);
    tracerArrowAmplitudeBottomTxt->brush().setStyle(Qt::SolidPattern);
    tracerArrowAmplitudeBottomTxt->setFont(font);
    tracerArrowAmplitudeBottomTxt->setPen(QPen(bgColor));
    tracerArrowAmplitudeBottomTxt->setColor(bgColor);
    tracerArrowAmplitudeBottomTxt->setBrush(QBrush(QColor(150, 255, 150)));
    tracerArrowAmplitudeBottomTxt->setPadding(QMargins(8, 4, 8, 4));

    tracerArrowFromRef = new QCPItemLine(ui->plot);
    tracerArrowFromRef->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowFromRef->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowFromRef->setPen(QPen(QColor(255, 255, 150), 4));
    tracerArrowFromRefTxt = new QCPItemText(ui->plot);
    tracerArrowFromRefTxt->setRoundCorners(5);
    tracerArrowFromRefTxt->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
    tracerArrowFromRefTxt->setTextAlignment(Qt::AlignLeft);
    tracerArrowFromRefTxt->brush().setStyle(Qt::SolidPattern);
    tracerArrowFromRefTxt->setFont(font);
    tracerArrowFromRefTxt->setPen(QPen(bgColor));
    tracerArrowFromRefTxt->setColor(bgColor);
    tracerArrowFromRefTxt->setBrush(QBrush(QColor(255, 255, 150)));
    tracerArrowFromRefTxt->setPadding(QMargins(8, 4, 8, 4));

//    traceLineTop->setVisible(false);
//    traceLineTop->start->setCoords(0, -DBL_MAX);
//    traceLineTop->end->setCoords(DBL_MAX, -DBL_MAX);
    cleanTracer();
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );

    lastDataTtime = 0;
    cleanDataTtime = PLOT_TIME_MAX_DEF;
}

/******************************************************************************************************************/
void MainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   if (infoModeLabel) {
       QFontMetricsF fm(infoModeLabel->font());
       qreal pixelsWide = fm.width(infoModeLabel->text());
//       infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
//       infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 64, 16);
       infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
   }
}

/******************************************************************************************************************/
void MainWindow::dataTerminalReadyChanged(bool dtr) {
    if (dtr) {
        serialPort->clear(QSerialPort::AllDirections);
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    }
}

/******************************************************************************************************************/
void MainWindow:: closePort() {
    cleanGraphs();
    ui->tabWidget->removeTab(1);
    ui->tabWidget->setCurrentIndex(0);
    if (logFile != nullptr) {
        if(logFile->isOpen()) {
            logFile->close();
            delete logFile;
            logFile = nullptr;
        }
    }
    // Clear the terminal
    on_clearTermButton_clicked();
    ui->menuWidgets->menuAction()->setVisible(false);
    if ((widgets != nullptr) && widgets->isVisible()) {
            widgets->hide();
    }
    updateTimer.stop();
//    ticksXTimer.stop();
    connected = false;
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    disconnect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    disconnect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    serialPort->close();                                                              // Close serial port
    delete serialPort;                                                                // Delete the pointer
    serialPort = nullptr;                                                                // Assign NULL to dangling pointer
    ui->connectButton->setText("Connect");                                            // Change Connect button text, to indicate disconnected
    ui->statusBar->setStyleSheet("background-color: SkyBlue;");
    ui->statusBar->showMessage("Disconnected!");                                      // Show message in status bar
    connected = false;                                                                // Set connected status flag to false
    plotting = false;                                                                 // Not plotting anymore
    receivedData.clear();                                                             // Clear received string
    noMsgReceivedData.clear();
    ui->stopPlotButton->setEnabled(false);                                            // Take care of controls
    ui->saveJPGButton->setEnabled(false);
    enableControls(true);
}

/******************************************************************************************************************/
/* Replot */
/******************************************************************************************************************/
void MainWindow::replot() {
    if(connected) {
        ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
        ui->plot->replot();
    }
}

///******************************************************************************************************************/
//void MainWindow::addTickX() {
//    QTime z(0, 0);
//    z = z.addSecs(qRound(ticksXTime.elapsed()/1000.0));
//    textTicker->addTick(dataPointNumber, z.toString("mm:ss"));
//}

/******************************************************************************************************************/
void MainWindow::unselectGraphs() {
    qDebug() << "<<<<<<<<<<<<< unselectGraphs >>>>>>>>>>>>>>";
//    if (ui->plot->selectedItems().size()) {
    cleanTracer();
    resetMouseWheelState();
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->setSelected(false);
    }
    selectedPlotId = -1;
    workingGraph = nullptr;
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->plot->deselectAll();
}

/******************************************************************************************************************/
/* Stop Plot Button */
/******************************************************************************************************************/
void MainWindow::on_stopPlotButton_clicked() {
    if (plotting) {
        // Stop plotting
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        // Stop updating plot timer
        updateTimer.stop();
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
    } else {
        // Start plotting
        // Start updating plot timer
        updateTimer.start();
        plotting = true;
        ui->stopPlotButton->setText("Stop Plot");
    }
    unselectGraphs();
//    workingGraph = nullptr;
//    cleanTracer("on_stopPlotButton_clicked");
//    resetMouseWheelState();
}

/******************************************************************************************************************/
bool MainWindow::isColor(QString str) {
    if (!str.isEmpty()) {
        // check if start with #, if yes --> color
        if (str.at(0) == "#") {
            str = str.remove(0, 1);
            QRegularExpression hexMatcher("^[0-9A-F]{6}$", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = hexMatcher.match(str);
            if (match.hasMatch()) {
                // Found hex string of length 6.
                return true;
            }
        }
    }
    return false;
}

/******************************************************************************************************************/
yaspGraph* MainWindow::getGraph(int id) {
    if (graphs.contains(id)) {
        return graphs[id];
    }
    // Create new Graph
//    qDebug() << "Create new Graph -> " << id;
    yaspGraph* graph = addGraph(id);
    return graph;
}

/******************************************************************************************************************/
void MainWindow::onNewPlotDataArrived(QStringList newData) {
    if (newData.size() > 1) {
        int plotId = newData.at(0).toInt();
        if ((plotId < 0) || (plotId > 9)) {
            qDebug() << " BAD PLOT ID : " << plotId << " --> " << newData;
            addMessageText(" BAD PLOT ID : " + QString::number(plotId) + " --> " + newData.join(" / "), "tomato");
            return;
        }
        yaspGraph* yGraph = getGraph(plotId);
        QString param1 = "";
        QString param2 = "";
        QString id = newData.at(0);
        if (newData.size() > 1) {
            if (newData.size() > 2) {
                param2 = newData.at(2);
            }
            param1 = newData.at(1);
        }
        if (isColor(param1)) {
            QPen pen = yGraph->plot()->pen();
            pen.setColor(QColor(param1));
            yGraph->plot()->setPen(pen);

        } else {
            if ((!param1.isEmpty()) && ( yGraph->plot()->name() != param1)) {
                 yGraph->plot()->setName(param1);
            }
        }
        if (isColor(param2)) {
             yGraph->plot()->setPen(QPen(QColor(param2)));
        } else {
            if ((!param2.isEmpty()) && ( yGraph->plot()->name() != param2)) {
                QPen pen = yGraph->plot()->pen();
                pen.setColor(QColor(param2));
                yGraph->plot()->setPen(pen);
            }
        }
//        qDebug() << "param1 : " << param1;
//        qDebug() << "param2 : " << param2;
        updateLabel(plotId, yGraph->infoStr());
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Slot for new data from serial port . Data is comming in QStringList and needs to be parsed */
/******************************************************************************************************************/
void MainWindow::onNewDataArrived(QStringList newData) {
    Q_ASSERT(newData.size() > 0);
    int plotId = newData.at(0).toInt();
    if ((plotId < 0) || (plotId > 9)) {
        addMessageText(" BAD DATA ID : " + QString::number(plotId) + " --> " + newData.join(" / "), "tomato");
        return;
    }
    yaspGraph* yGraph = getGraph(plotId);
    if (true) {
        int dataListSize = newData.size();                                                    // Get size of received list
//        qDebug() << "NEW DATA : " << plotId << " / " << dataListSize << " --> " << newData;
        dataPointNumber++;
        if (dataListSize == 3) {
            lastDataTtime = newData[1].toDouble()/1000000.0;
            if (lastDataTtime > cleanDataTtime) {
                // Clean graphs data
                cleanGraphsBefore(lastDataTtime);
            }
            double val = newData[2].toDouble();
            val *= yGraph->mult();
            val += yGraph->offset();
            // Add data to graphs according plot Id
            QCPGraph* plot = yGraph->plot();
            QString plotInfoStr = " val: ";
            plotInfoStr += QString::number(val);
            plotInfoStr +=  + " offset: ";
            plotInfoStr += QString::number(yGraph->offset());
            plotInfoStr +=  + " mult: ";
            plotInfoStr += QString::number(yGraph->mult());
            updateLabel(plotId, plotInfoStr);
            plot->addData(lastDataTtime, val );
//            qDebug() << "ADD DATA : " << plotId << " --> " << time << " / " << val;
//            pointTime.insert(dataPointNumber, ticksXTime.elapsed());
//            yGraph->updateRefLine(dataPointNumber);
//            ui->statusBar->showMessage("Points: " + QString::number(dataPointNumber));
        } else {
            qDebug() << "------------> BAD DATA : " << plotId << " / " << dataListSize << " --> " << newData;
        }
    }
}

/******************************************************************************************************************/
void MainWindow::addMessageText(QString data, QString color) {
    //if (data.isEmpty()) return;
    if(data.trimmed().isEmpty()) return;
    data = data.replace("\n", "");
    data = data.replace("\r", "\n");
    QString string = QUrl::fromPercentEncoding(data.toLatin1());
    QString html = "<span style=\"color:"+color+";font-weight:bold\">"+string+"</span>";
    if (ui->scrollButton->isChecked()) {
        ui->receiveTerminal->moveCursor (QTextCursor::End);
        ui->receiveTerminal->appendHtml(html);
        ui->receiveTerminal->moveCursor (QTextCursor::End);
    } else {
        const QTextCursor old_cursor = ui->receiveTerminal->textCursor();
        const int old_scrollbar_value = ui->receiveTerminal->verticalScrollBar()->value();
        const bool is_scrolled_down = old_scrollbar_value == ui->receiveTerminal->verticalScrollBar()->maximum();
        // Move the cursor to the end of the document.
        ui->receiveTerminal->moveCursor(QTextCursor::End);
        // Insert the text at the position of the cursor (which is the end of the document).
        ui->receiveTerminal->appendHtml(html);
        if (old_cursor.hasSelection() || !is_scrolled_down) {
            // The user has selected text or scrolled away from the bottom: maintain position.
            ui->receiveTerminal->setTextCursor(old_cursor);
            ui->receiveTerminal->verticalScrollBar()->setValue(old_scrollbar_value);
        } else {
            // The user hasn't selected any text and the scrollbar is at the bottom: scroll to the bottom.
            ui->receiveTerminal->moveCursor(QTextCursor::End);
            ui->receiveTerminal->verticalScrollBar()->setValue(ui->receiveTerminal->verticalScrollBar()->maximum());
        }
    }
}

/******************************************************************************************************************/
bool MainWindow::checkEndMsgMissed( char cc) {
    if ( cc == START_MSG) {
        // Houps, seems we missed the END_MSG (maybe an external reset of the device !)
        // Just start MSG scanning again
        //addMessageText(data, "red");
        qDebug() << "MISSED IN_MESSAGE !!";
        QString recoverStr = data;
        int index = recoverStr.indexOf(START_MSG);
        QString msg = recoverStr.left(index);
        recoverStr = recoverStr.right(index);
        addMessageText(msg, "orange");
        addMessageText("-> DATA RECOVERED : " + recoverStr, "red");
        receivedData.clear();
        STATE = IN_MESSAGE;
        return true;
     } else if ( cc == PLOT_MSG) {
        // Houps, seems we missed the END_MSG (maybe an external reset of the device !)
        // Just start PLOT_MSG scanning again
        qDebug() << "MISSED IN_PLOT_MSG !!";
        int index = data.indexOf(PLOT_MSG);
        QString msg = data.left(index);
        QString recoverStr = data.mid(index);
        addMessageText(msg, "orange");
        addMessageText("-> DATA RECOVERED : " + recoverStr, "red");
        receivedData.clear();
        STATE = IN_PLOT_MSG;
        return true;
    }
    return false;
}

/******************************************************************************************************************/
bool MainWindow::isNumericChar(char cc) {
//  Fixed Strange Behavior !!
//    From ---> https://en.cppreference.com/w/cpp/string/byte/isdigit
//    Like all other functions from <cctype>, the behavior of std::isdigit is undefined if the argument's
//    value is neither representable as unsigned char nor equal to EOF. To use these functions safely with
//    plain chars (or signed chars), the argument should first be converted to unsigned char:
    unsigned char ucc = static_cast<unsigned char>(cc);
    return isdigit( ucc) || isspace( ucc) || ( ucc == '-') || ( ucc == '.') || (cc == SPACE_MSG);
}

/******************************************************************************************************************/
/* Read data for inside serial port */
/******************************************************************************************************************/
void MainWindow::readData() {
    if (serialPort->bytesAvailable()) {                                                    // If any bytes are available
        data = serialPort->readAll();         // Read all data in QByteArray
//        if (!updateTimer.isActive()) return;
        if(!data.isEmpty()) {                                                             // If the byte array is not empty
            char *temp = data.data();
            // Get a '\0'-terminated char* to the data
            for(int i = 0; temp[i] != '\0'; i++) {                                        // Iterate over the char*
                char cc = temp[i];
                switch(STATE) {                                                           // Switch the current state of the message
                case WAIT_START:                                                          // If waiting for start [$], examine each char
                    if( cc == START_MSG) {                                            // If the char is $, change STATE to IN_MESSAGE
                        STATE = IN_MESSAGE;
                        receivedData.clear();                                             // Clear temporary QString that holds the message
                        break;                                                            // Break out of the switch
                    }
                    if( cc == PLOT_MSG) {                                            // If the char is $, change STATE to IN_MESSAGE
                        STATE = IN_PLOT_MSG;
                        receivedData.clear();                                             // Clear temporary QString that holds the message
                        break;                                                            // Break out of the switch
                    }
                    if ((STATE != IN_MESSAGE) && (STATE != IN_PLOT_MSG)) {
                        if (checkEndMsgMissed(cc)) {
                            break;
                        } else if ( cc == '\n') {
                            if (!noMsgReceivedData.isEmpty()) {
                                addMessageText(noMsgReceivedData, "blue");
                            }
                            noMsgReceivedData.clear();
                        } else {
                            // Check if printable char
//                            if ( !(cc < 0x20 || cc > 127)) {
                            if (cc > 0x1F) {
                                noMsgReceivedData.append( cc);
                            }
                        }
                    }

                    break;
                case IN_MESSAGE:                                                          // If state is IN_MESSAGE
                    if( cc == END_MSG) {                                              // If char examined is ;, switch state to END_MSG
                        STATE = WAIT_START;
                        QStringList incomingData = receivedData.split(SPACE_MSG);               // Split string received from port and put it into list
                        emit newData(incomingData);                                       // Emit signal for data received with the list
                        break;
                    } else if (checkEndMsgMissed(cc)) {
                        break;
                    } else {
                        if (isNumericChar(cc)) {            // If examined char is a digit, and not '$' or ';', append it to temporary string
                            receivedData.append( cc);
                        }
                    }
                    break;
                case IN_PLOT_MSG:                                                          // If state is IN_MESSAGE
                    if( cc == END_MSG) {                                              // If char examined is ;, switch state to END_MSG
                        STATE = WAIT_START;
//                        qDebug() << "receivedPlot: " << receivedData;
                        QStringList incomingData = receivedData.split(SPACE_MSG);               // Split string received from port and put it into list
                        emit newPlotData(incomingData);                                       // Emit signal for data received with the list
                        break;
                    } else if (checkEndMsgMissed(cc)) {
                        break;
                    } else {
                        receivedData.append( cc);
                    }
                    break;
                default:
                    qDebug() << "++++ " << data;
                    break;
                }
            }
        }
    }
}

/******************************************************************************************************************/
/* Save a JPG image of the plot to current EXE directory */
/******************************************************************************************************************/
void MainWindow::on_saveJPGButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot"),
                               "",
                               tr("Images (*.jpg)"));
    qDebug() << "Save JPEG : " << fileName;
    ui->plot->saveJpg(fileName);
}

/******************************************************************************************************************/
/* Reset the zoom of the plot to the initial values */
/******************************************************************************************************************/
void MainWindow::on_resetPlotButton_clicked() {
    unselectGraphs();
    plotTimeInSeconds = PLOT_TIME_DEF;
    ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);

    ui->spinDisplayTime->setValue(plotTimeInSeconds);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
//    cleanTracer("on_resetPlotButton_clicked");
}

/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinDisplayTime_valueChanged(double arg1) {
    plotTimeInSeconds = arg1;
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::shiftPlot(int pY) {
    double posY = ui->plot->yAxis->pixelToCoord(pY);
    Q_ASSERT(workingGraph);
    if (startShiftPlot) {
        lastPosY = posY;
        lastY = pY;
        startShiftPlot = false;
    } else {
        double offset = posY - lastPosY;
        workingGraph->setOffset(offset);
        ui->plot->replot();
        lastPosY = posY;
        lastY = pY;
    }
}

/******************************************************************************************************************/
void MainWindow::scalePlot(double scale) {
//    if (scale == 0) return;
//    if (startScalePlot) {
//        scaleMult = 1.0;
//        startScalePlot = false;
//    }
    Q_ASSERT(workingGraph);
//    double scale = 1 + ((double)numDeg / 1000.0);
    workingGraph->setMult(scale);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::cleanTracer() {
    qDebug() << "========================== cleanTracer ==========================";
//    if (mouseState == mouseDoMesure) {
        ui->plot->setCursor(Qt::ArrowCursor);
        tracer->setVisible(false);
        ShowPlotsExceptWG(true);

        traceLineBottom->start->setCoords(0, DBL_MAX);
        traceLineBottom->end->setCoords(DBL_MAX, DBL_MAX);
        traceLineBottom->setVisible(false);
        traceLineTop->setVisible(false);
        traceLineTop->start->setCoords(0, -DBL_MAX);
        traceLineTop->end->setCoords(DBL_MAX, -DBL_MAX);
        tracerArrowAmplitude->setVisible(false);
        tracerArrowAmplitudeTop->setVisible(false);
        tracerArrowAmplitudeBottom->setVisible(false);
        tracerArrowAmplitudeTxt->setVisible(false);
        tracerArrowAmplitudeTopTxt->setVisible(false);
        tracerArrowAmplitudeBottomTxt->setVisible(false);
        tracerArrowFromRef->setVisible(false);
        tracerArrowFromRefTxt->setVisible(false);
        tracerRect->setVisible(false);
        QList<QCPItemLine*>::iterator lI;
        lI = tracerHLinesRef.end();
        while(lI != tracerHLinesRef.begin()) {
          --lI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*lI);
        }
        tracerHLinesRef.clear();
        lI = tracerHLinesTracer.end();
        while(lI != tracerHLinesTracer.begin()) {
          --lI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*lI);
        }
        tracerHLinesTracer.clear();
        QList<QCPItemText*>::iterator tI = tracerHLinesRefInfo.end();
        while(tI != tracerHLinesRefInfo.begin()) {
          --tI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*tI);
        }
        tracerHLinesRefInfo.clear();
        tI = tracerHLinesTracerInfo.end();
        while(tI != tracerHLinesTracerInfo.begin()) {
          --tI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*tI);
        }
        tracerHLinesTracerInfo.clear();
//    }
    resetMouseWheelState();
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotShiftAction() {
    qDebug() << "doMenuPlotShiftAction: " << contextMenu->property("id");
//    Q_ASSERT(contextMenu);
    resetMouseWheelState();
//    int plotId = contextMenu->property("id").toInt();
//    yaspGraph* yGraph = graphs[plotId];
//    Q_ASSERT(yGraph);
//    workingGraph = yGraph;
    Q_ASSERT(workingGraph);
    mouseState = mouseShift;
    infoModeLabel->setText(workingGraph->plot()->name() + " -> SHIFT MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(workingGraph->plot()->pen().color());
//    QFontMetricsF fm(infoModeLabel->font());
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
}

/******************************************************************************************************************/
void MainWindow::ShowPlotsExceptWG(bool show) {
    if (workingGraph == nullptr) return;
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        if (workingGraph == yGraph) continue;
        yGraph->toggleTracerVisibility(show);
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotMeasureAction() {
    qDebug() << "doMenuPlotMeasureAction: " << contextMenu->property("id") << " mouseState: " << mouseState;
    Q_ASSERT(contextMenu);
    Q_ASSERT(workingGraph);
    if (mouseState == mouseDoMesure) {
        cleanTracer();
        mouseState = mouseNone;
        plotLabelSelected(true);
    } else {
        qDebug() << "Init Tracer !";
        mouseState = mouseDoMesure;
        ShowPlotsExceptWG(false);
        measureMult = workingGraph->mult();
        ui->plot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag );
        infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        plotLabelSelected(true);
//        QFontMetricsF fm(infoModeLabel->font());
//        qreal pixelsWide = fm.width(infoModeLabel->text());
//        infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
        ui->plot->setCursor(Qt::CrossCursor);
        tracer->setVisible(true);
        tracer->setGraph(workingGraph->plot());
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuCloseAction(bool) {
    qDebug() << "doMenuCloseAction: " << contextMenu->property("id");
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotColorAction() {
    qDebug() << "doMenuPlotColorAction: " << contextMenu->property("id");
//    Q_ASSERT(contextMenu);
    resetMouseWheelState();
//    int plotId = contextMenu->property("id").toInt();
//    yaspGraph* yGraph = graphs[plotId];
//    Q_ASSERT(yGraph);
//    qDebug() << "doMenuPlotColorAction: " << plotId;
    Q_ASSERT(workingGraph);
    infoModeLabel->setText(workingGraph->plot()->name() + " --> COLOR MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(workingGraph->plot()->pen().color());
//    QFontMetricsF fm(infoModeLabel->font());
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    QColor color = QColorDialog::getColor(Qt::white, nullptr, "plot color");
    if (color.isValid()) {
        qDebug() << "doMenuPlotColorAction: " << color;
        QPen pen = workingGraph->plot()->pen();
        pen.setColor(color);
        workingGraph->plot()->setPen(pen);
        workingGraph->setSelected(true);
//        workingGraph->info()->setColor(color);
//        workingGraph->info()->setSelectedPen(QPen(color));
//        workingGraph->info()->setSelectedColor(color);
//        workingGraph->rLine()->setPen(QPen(pen.color()));
    }
    resetMouseWheelState();
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotResetAction() {
    qDebug() << "doMenuPlotScaleAction: " << contextMenu->property("id");
    resetMouseWheelState();
    Q_ASSERT(workingGraph);
    workingGraph->reset();
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotScaleAction() {
//    Q_ASSERT(contextMenu);
    qDebug() << "doMenuPlotScaleAction: " << contextMenu->property("id");
    resetMouseWheelState();
//    int plotId = contextMenu->property("id").toInt();
//    yaspGraph* yGraph = graphs[plotId];
//    Q_ASSERT(yGraph);
//    workingGraph = yGraph;
    Q_ASSERT(workingGraph);
    infoModeLabel->setText(workingGraph->plot()->name() + " --> SCALE MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(workingGraph->plot()->pen().color());
//    QFontMetricsF fm(infoModeLabel->font());
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    wheelState = wheelScalePlot;
//    startScalePlot = true;
    ui->plot->setInteraction(QCP::iRangeZoom, false);
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotShowHideAction() {
    qDebug() << "doMenuPlotShowHideAction: " << contextMenu->property("id");
//    Q_ASSERT(contextMenu);
    resetMouseWheelState();
//    int plotId = contextMenu->property("id").toInt();
//    yaspGraph* yGraph = graphs[plotId];
//    Q_ASSERT(yGraph);
    Q_ASSERT(workingGraph);
    if (workingGraph->plot()->visible()) {
        workingGraph->plot()->setVisible(false);
        workingGraph->rLine()->setVisible(false);
        plotShowHideAction->setText("show");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
        infoModeLabel->setText(workingGraph->plot()->name() + " --> HIDE MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
    } else {
        workingGraph->plot()->setVisible(true);
        workingGraph->rLine()->setVisible(true);
        plotShowHideAction->setText("Hide");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-hide-48.png"));
        infoModeLabel->setText(workingGraph->plot()->name() + " --> SHOW MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
    }
//    QFontMetricsF fm(infoModeLabel->font());
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    ui->plot->replot();
}

//*******************************************************************************************/
void MainWindow::saveDataPlot(yaspGraph* yGraph) {
    QCPGraph* g = yGraph->plot();
    if (!g->visible()) return;
    infoModeLabel->setText(yGraph->plot()->name() + " --> SAVE MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(yGraph->plot()->pen().color());
    QString plotName = g->name();
    if (logData == nullptr) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Log Plot"),
                                   plotName,
                                   tr("Data (*.csv)"));
        qDebug() << "Log DATA : " << fileName;
        logData = new QFile(fileName);
        if (!logData->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     logData->errorString());
            logData = nullptr;
            return;
        }
        qDebug() << "Log DATA Opened : " << logData->fileName();
        streamData.setDevice(logData);
        yGraph->save(streamData);
    }
   qDebug() << "Close Log DATA : " << logData->fileName();
   logData->close();
   delete logData;
   logData = nullptr;
}

/******************************************************************************************************************/
void MainWindow::saveSelectedGraph() {
    qDebug() << "saveSelectedGraph: " << contextMenu->property("id");
    Q_ASSERT(selectedPlotId >= 0);
    yaspGraph* yGraph = graphs[selectedPlotId];
    Q_ASSERT(yGraph);
    saveDataPlot(yGraph);
    resetMouseWheelState();
}

/******************************************************************************************************************/
void MainWindow::saveAllGraphs() {
}

/******************************************************************************************************************/
void MainWindow::messageSent(QString str) {
    addMessageText("--> " + str, "gray");
}

/******************************************************************************************************************/
void MainWindow::updateTracer(int pX) {
//    qDebug() << "updateTracer " << tracer << " / " << pX;
    if (mouseState == mouseDoMesure) {
        Q_ASSERT(tracer);
//        cleanTracer();
        QList<QCPItemLine*>::iterator lI;
        lI = tracerHLinesRef.end();
        while(lI != tracerHLinesRef.begin()) {
          --lI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*lI);
        }
        tracerHLinesRef.clear();
        lI = tracerHLinesTracer.end();
        while(lI != tracerHLinesTracer.begin()) {
          --lI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*lI);
        }
        tracerHLinesTracer.clear();
        QList<QCPItemText*>::iterator tI = tracerHLinesRefInfo.end();
        while(tI != tracerHLinesRefInfo.begin()) {
          --tI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*tI);
        }
        tracerHLinesRefInfo.clear();
        tI = tracerHLinesTracerInfo.end();
        while(tI != tracerHLinesTracerInfo.begin()) {
          --tI;
//            QCPItemLine* item = dynamic_cast<QCPItemLine*>(lI)
          ui->plot->removeItem(*tI);
        }
        tracerHLinesTracerInfo.clear();

        double plotWidth = ui->plot->xAxis->range().size();
        double plotHeight = ui->plot->yAxis->range().size();
        double plotVCenter = ui->plot->yAxis->range().center();
//        double plotTop = ui->plot->yAxis->range().upper;
//        double plotBottom = ui->plot->yAxis->range().lower;
        double space = plotWidth / 800;
        double coordX = ui->plot->xAxis->pixelToCoord(pX);
        // get tracer origin
        tracer->setGraphKey(tracerStartKey);
        tracer->updatePosition();
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
//        QPointF tracerPos = tracer->position->pixelPosition();
        double endX = tracer->position->key();
        double endY = tracer->position->value();
        traceLineBottom->setVisible(true);
        traceLineTop->setVisible(true);
        tracerRect->setVisible(true);

        if (traceLineBottom->start->coords().y() > endY) {
            traceLineBottom->start->setCoords(0, endY);
            traceLineBottom->end->setCoords(DBL_MAX, endY);
        }
        if (traceLineTop->start->coords().y() < endY) {
            traceLineTop->start->setCoords(0, endY);
            traceLineTop->end->setCoords(DBL_MAX, endY);
        }
        double rh = plotHeight * 0.35;
        double tracerRectTop = plotVCenter + rh;
        double tracerRectBottom = plotVCenter - rh;
        double tracerRectLeft = endX - (plotWidth/4);
        double tracerRectRight = endX + (plotWidth/4);
        tracerRect->topLeft->setCoords(tracerRectLeft, tracerRectTop);
        tracerRect->bottomRight->setCoords(tracerRectRight, tracerRectBottom);

        // Vertical measures
        double amplitude = traceLineTop->start->coords().y() - traceLineBottom->start->coords().y();
        double deltaTop = traceLineTop->start->coords().y() - endY;
        double deltaBottom = endY - traceLineBottom->start->coords().y();
        double ref = workingGraph->rLine()->start->coords().y();
        double deltaFromRef = endY - ref;
        double space0 = 15.0;
        double space1 = 30.0;
        QString tracerInfo("Ref: %1 Delta+: %2 Delta-: %3 Min: %4 Max: %5 Amplitude: %6 Raw Amplitude: %7");
        tracerInfo =  tracerInfo.arg(endY)
                .arg(deltaTop)
                .arg(deltaBottom)
                .arg(traceLineBottom->start->coords().y())
                .arg(traceLineTop->start->coords().y())
                .arg(amplitude)
                .arg(amplitude / workingGraph->mult());
        ui->statusBar->setStyleSheet("background-color: lightgreen;");
        ui->statusBar->showMessage(tracerInfo);

        double ampTracerArrowSY = traceLineBottom->start->coords().y();
        double ampTracerArrowEY = traceLineTop->end->coords().y();
        double mult = workingGraph->mult();

        if (qFuzzyCompare(amplitude, 0)) {
            tracerArrowAmplitude->setVisible(false);
            tracerArrowAmplitudeTxt->setVisible(false);
        } else {
            tracerArrowAmplitude->setVisible(true);
            tracerArrowAmplitude->start->setCoords(coordX - space0*space, ampTracerArrowSY);
            tracerArrowAmplitude->end->setCoords(coordX - space0*space, ampTracerArrowEY);
            tracerArrowAmplitudeTxt->setFont(QFont(font().family(), 8));
            tracerArrowAmplitudeTxt->setVisible(true);
            if (qFuzzyCompare(mult, 1.0)) {
                tracerArrowAmplitudeTxt->setText(QString::number(amplitude));
            } else {
                tracerArrowAmplitudeTxt->setText(QString::number(amplitude) + " ["
                                                 + QString::number(amplitude/mult) + "]");
            }
            tracerArrowAmplitudeTxt->position->setCoords(coordX - (space0+space1)*space,
                                                         (ampTracerArrowSY + ampTracerArrowEY)/2.0);
        }
        if (qFuzzyCompare(deltaTop, 0)) {
            tracerArrowAmplitudeTop->setVisible(false);
            tracerArrowAmplitudeTopTxt->setVisible(false);
        } else {
            tracerArrowAmplitudeTop->setVisible(true);
            tracerArrowAmplitudeTop->start->setCoords(coordX + space0*space, endY);
            tracerArrowAmplitudeTop->end->setCoords(coordX + space0*space, ampTracerArrowEY);
            tracerArrowAmplitudeTopTxt->setFont(QFont(font().family(), 8));
            tracerArrowAmplitudeTopTxt->setVisible(true);
            if (qFuzzyCompare(mult, 1.0)) {
                tracerArrowAmplitudeTopTxt->setText(QString::number(deltaTop));
            } else {
                tracerArrowAmplitudeTopTxt->setText(QString::number(deltaTop) + " ["
                                                 + QString::number(deltaTop/mult) + "]");
            }
            tracerArrowAmplitudeTopTxt->position->setCoords(coordX + space1*space,
                                                         (endY + ampTracerArrowEY)/2.0);
        }
        if (qFuzzyCompare(deltaBottom, 0)) {
            tracerArrowAmplitudeBottom->setVisible(false);
            tracerArrowAmplitudeBottomTxt->setVisible(false);
        } else {
            tracerArrowAmplitudeBottom->setVisible(true);
            tracerArrowAmplitudeBottom->start->setCoords(coordX + space0*space, ampTracerArrowSY);
            tracerArrowAmplitudeBottom->end->setCoords(coordX + space0*space, endY);
            tracerArrowAmplitudeBottomTxt->setFont(QFont(font().family(), 8));
            tracerArrowAmplitudeBottomTxt->setVisible(true);
            if (qFuzzyCompare(mult, 1.0)) {
                tracerArrowAmplitudeBottomTxt->setText(QString::number(deltaBottom));
            } else {
                tracerArrowAmplitudeBottomTxt->setText(QString::number(deltaBottom) + " ["
                                                 + QString::number(deltaBottom/mult) + "]");
            }
            tracerArrowAmplitudeBottomTxt->position->setCoords(coordX + space1*space,
                                                         (ampTracerArrowSY + endY)/2.0);
        }

        if (qFuzzyCompare(deltaFromRef, 0)) {
            tracerArrowFromRef->setVisible(false);
            tracerArrowFromRefTxt->setVisible(false);
        } else {
            tracerArrowFromRef->setVisible(true);
            tracerArrowFromRef->start->setCoords(coordX - space1*space, ref);
            tracerArrowFromRef->end->setCoords(coordX - space1*space, endY);
            tracerArrowFromRefTxt->setFont(QFont(font().family(), 8));
            tracerArrowFromRefTxt->setVisible(true);
            if (qFuzzyCompare(mult, 1.0)) {
                tracerArrowFromRefTxt->setText(QString::number(deltaFromRef));
            } else {
                tracerArrowFromRefTxt->setText(QString::number(deltaFromRef) + " ["
                                                 + QString::number(deltaFromRef/mult) + "]");
            }
            tracerArrowFromRefTxt->position->setCoords(coordX - (space0+space1)*space,
                                                         (ref + endY)/2.0);
        }
        // Horizontal measures
        QSharedPointer<QCPGraphDataContainer> gData = workingGraph->plot()->data();
        QCPDataContainer<QCPGraphData>::const_iterator itStart = gData->findBegin(tracerRectLeft, true);
        QCPDataContainer<QCPGraphData>::const_iterator itEnd = gData->findBegin(tracerRectRight, true);
//        qDebug() << "-----------------------------";
        lastTracerXValueRef = itStart->key;
        lastTracerXValueTracer = itStart->key;
        double deltaMin = space * 20;
//        qDebug() << "itStart: " <<  itStart->key << " itEnd: " <<  itEnd->key
//                 << " lastTracerXValue: " << lastTracerXValueRef << " deltaMin: " << deltaMin;
        for (QCPDataContainer<QCPGraphData>::const_iterator it = itStart; it != itEnd; ++it) {
            if (compareDouble( it->value, ref, 0)) {
                double deltaRef = it->key - lastTracerXValueRef;
//                qDebug() << "deltaRef: " << deltaRef;
                if (deltaRef > deltaMin) {
                    QCPItemLine* line = new QCPItemLine(ui->plot);
                    tracerHLinesRef.append(line);
                    // Orange
//                    QColor col(255,165,0);
                    QColor col(255,140,0);
                    QPen pen(col, 2);
                    pen.setStyle(Qt::DotLine);
                    line->setPen(pen);
                    line->start->setCoords(it->key, it->value);
                    line->end->setCoords(it->key, tracerRectBottom);
//                    qDebug() << "ref: " << it->key << " / " << it->value;
                    if (tracerHLinesRef.size() > 1) {
                        // Add Time info between lines
                        QCPItemText* info = new QCPItemText(ui->plot);
                        info->setText(QString::number(deltaRef));
                        info->setFont(QFont(font().family(), 8));
                        info->setPositionAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                        info->setTextAlignment(Qt::AlignCenter);
                        info->setRotation(45);
                        info->setRoundCorners(5);
                        info->setPen(QPen(Qt::black));
                        info->setColor(Qt::black);
                        info->setBrush(QBrush(col));
                        info->setPadding(QMargins(4,2,4,2));
                        info->setVisible(true);
                        info->position->setCoords(lastTracerXValueRef + deltaRef/2.0, tracerRectBottom);
                        tracerHLinesRefInfo.append(info);
                    }
                }
                lastTracerXValueRef = it->key;
            }
           if (compareDouble( it->value, endY, 0)) {
                double deltaTracer = it->key - lastTracerXValueTracer;
//                qDebug() << "deltaTracer: " << deltaTracer;
                if (deltaTracer > deltaMin) {
//                    qDebug() << "tracer: " << it->key << " / " << it->value << " deltaMin: " << deltaMin;
                    QCPItemLine* line = new QCPItemLine(ui->plot);
                    tracerHLinesTracer.append(line);
                    // Blue
//                    QColor col(0,191,255);
                    QColor col(30,144,255);
//                    QColor col(138,43,226);
                    QPen pen(col, 2);
                    pen.setStyle(Qt::DotLine);
                    line->setPen(pen);
                    line->start->setCoords(it->key, it->value);
                    line->end->setCoords(it->key, tracerRectTop);
                    if (tracerHLinesTracer.size() > 1) {
                        // Add Time info between lines
                        QCPItemText* info = new QCPItemText(ui->plot);
                        info->setText(QString::number(deltaTracer));
                        info->setFont(QFont(font().family(), 8));
                        info->setPositionAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                        info->setTextAlignment(Qt::AlignCenter);
                        info->setRotation(-45);
                        info->setRoundCorners(5);
                        info->setPen(QPen(Qt::black));
                        info->setColor(Qt::black);
                        info->setBrush(QBrush(col));
                        info->setPadding(QMargins(4,2,4,2));
                        info->setVisible(true);
                        info->position->setCoords(lastTracerXValueTracer + deltaTracer/2.0, tracerRectTop);
                        tracerHLinesTracerInfo.append(info);
                    }
                }
                lastTracerXValueTracer = it->key;
            }
        }

        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Prints coordinates of mouse pointer in status bar on mouse release */
/******************************************************************************************************************/
void MainWindow::onMouseMoveInPlot(QMouseEvent *event) {
    double xx = ui->plot->xAxis->pixelToCoord(event->x());
    double yy = ui->plot->yAxis->pixelToCoord(event->y());
    QString coordinates("X: %1 Y: %2 Points:%3");
    coordinates = coordinates.arg(xx).arg(yy).arg(dataPointNumber);
    ui->statusBar->setStyleSheet("background-color: SkyBlue;");
    switch (mouseState) {
        case mouseMove: {
            if (wheelState == wheelZoom) {
                ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
            }
            ui->statusBar->showMessage(coordinates);
            }
            break;
    case mouseShift: {
            if (mouseButtonState == Qt::LeftButton) {
                ui->plot->setInteractions(QCP::iRangeZoom | QCP::iSelectItems );
                shiftPlot(event->y());
                QString msg = "Moving PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->offset());
                ui->statusBar->showMessage(msg);
            } else {
                ui->statusBar->showMessage(coordinates);
            }
            }
            break;
        case mouseDoMesure:
//            ui->plot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag );
            updateTracer(event->pos().x());
            break;
        default:
            ui->statusBar->showMessage(coordinates);
            break;
    }

//    if (mouseState == mouseMove) {
//    } else if (mouseState == mouseShift) {
//    } else if (mouseState == mouseDoMesure) {
//    }
}

/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event) {
    Q_UNUSED(event)
    ui->statusBar->showMessage("release");
    switch (mouseState) {
    case mouseDoMesure:
        break;
    case mouseShift:
    case mouseMove:
        startShiftPlot = false;
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
        break;
    default:
        break;
    }
//    mousePressed = false;
    mouseButtonState = Qt::NoButton;

//    if (plotting) {
////        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
//    } else {
////        if (tracer) {
////            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
////            tracer->blockSignals(false);
////        } else {
////            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems  | QCP::iSelectAxes | QCP::iSelectPlottables);
////        }
//    }
}

/******************************************************************************************************************/
void MainWindow::resetMouseWheelState() {
    qDebug() << "resetMouseWheelState: " << mouseState;
    mouseState = mouseNone;
    wheelState = wheelNone;
    startShiftPlot = false;
    mouseButtonState = Qt::NoButton;
    infoModeLabel->setText("");
    infoModeLabel->setVisible(false);
    infoModeLabel->setColor(QColor(0, 255, 255));
//    QFontMetricsF fm(infoModeLabel->font());
//    qreal pixelsWide = fm.width(infoModeLabel->text());
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::onMouseWheelInPlot(QWheelEvent *event) {
//     "onMouseWheelInPlot: " << mouseButtonState;
    if (mouseButtonState == Qt::RightButton) {
//        double xx = ui->plot->xAxis->pixelToCoord(event->posF().x());
//        double yy = ui->plot->yAxis->pixelToCoord(event->posF().y());
        QCPRange range = ui->plot->xAxis->range();
        double spd = ui->spinDisplayTime->value();
        // change spinDisplayTime
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
        QPoint numDegrees = event->angleDelta();
        if (numDegrees.y() == 0) return;
        double inc = plotTimeInSeconds / 100;
        if (numDegrees.y() > 0) {
            ui->spinDisplayTime->setValue(ui->spinDisplayTime->value() + inc);
        } else {
            ui->spinDisplayTime->setValue(ui->spinDisplayTime->value() - inc);
        }
        if (plotting) {
            ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
        } else {
            double newSpd = ui->spinDisplayTime->value();
            double ratio = newSpd/spd;
            QCPRange newRange = range * ratio;
//            qDebug() << xx << " = " << spd << " / " << newSpd << " -> " << ratio << " -> " <<  range << " / " << newRange;
            ui->plot->xAxis->setRange(newRange);
            ui->plot->replot();
        }
    } else {
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        if (wheelState == wheelScalePlot) {
            QPoint numDegrees = event->angleDelta();
            int nY = numDegrees.y();
            if (nY != 0) {
            double scale = 1 + (static_cast<double>(nY) / 1000.0);
            scalePlot(scale);
            QString msg = "Scaling PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->mult());
            ui->statusBar->showMessage(msg);
            event->accept();
            }
        } else {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
        }
    }
}

///******************************************************************************************************************/
//void MainWindow::onMouseDoubleClickInPlot(QMouseEvent* event) {
//}

/******************************************************************************************************************/
void MainWindow::onMousePressInPlot(QMouseEvent *event) {
    qDebug() << "onMousePressInPlot " << event->button()
             << " mouseState: " << mouseState
             << " menu: " << contextMenu->isVisible();
    mouseButtonState = event->button();
    switch (mouseState) {
    case mouseShift:
        startShiftPlot = true;
        break;
//    case mouseDoMesure:
////        tracer->blockSignals(true);
//        break;
    default:
//        if (contextMenu->isVisible()) {
//            contextMenu->hide();
//        }
        break;
    }
//    if (tracer) {
//
//        if (event->button() == Qt::MiddleButton) {
//        }
//    }
//    // if an axis is selected, only allow the direction of that axis to be dragged
//    // if no axis is selected, both directions may be dragged
//    if (ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//        qDebug() << "xAxis";
//        ui->plot->axisRect()->setRangeDrag(ui->plot->xAxis->orientation());
//    } else if (ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//        ui->plot->axisRect()->setRangeDrag(ui->plot->yAxis->orientation());
//        qDebug() << "yAxis";
//    } else {
//        qDebug() << "xAxis yAxis";
//        ui->plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
//    }
}

/******************************************************************************************************************/
void MainWindow::plotContextMenuRequest(QPoint pos) {
    if (contextMenu == nullptr) return;
    contextMenu->popup(ui->plot->mapToGlobal(pos));
}

/******************************************************************************************************************/
/* Shows a window with instructions */
/******************************************************************************************************************/
void MainWindow::on_actionHow_to_use_triggered() {
    if (helpWindow == nullptr) {
        helpWindow = new HelpWindow(this);
        helpWindow->setWindowTitle("How to use this application");
        helpWindow->show();
    }
}

/******************************************************************************************************************/
void MainWindow::on_sendLine_returnPressed() {
    QString data = ui->sendLine->text();
    if (data.isEmpty()) return; // Nothing to send
    data += "\n";
    // Send data
    qint64 result = serialPort->write(data.toLocal8Bit());
    if (result != -1) {
        // If data was sent successfully, clear line edit and echo sent data
        addMessageText("&rarr;&nbsp;" + data + "\n", "lightblue");
        ui->sendLine->clear();
    }
}

/******************************************************************************************************************/
void MainWindow::on_clearTermButton_clicked() {
    ui->receiveTerminal->clear();
    ui->receiveTerminal->moveCursor (QTextCursor::End);
}

/******************************************************************************************************************/
void MainWindow::on_actionShowWidgets_triggered()
{
    if (widgets == nullptr) {
        widgets = new DialogWidgets(serialPort, this);
        connect(widgets, SIGNAL( messageSent(QString)), this, SLOT(messageSent(QString)));
        widgets->setWindowTitle("Widgets");
        widgets->show();
    }
    else {
        if (widgets->isHidden()) {
            widgets->show();
        } else {
            widgets->hide();
        }
    }
}

/******************************************************************************************************************/
void MainWindow::on_bgColorButton_pressed() {
    bgColor = QColorDialog::getColor(bgColor, this, "Select Background Color");
    if (bgColor.isValid()) {
        ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");
        ui->plot->setBackground(QBrush(bgColor));   // Background for the plot area
        if (tracer) {
            if (bgColor.lightness() > 128) {
                tracer->setPen(QPen(Qt::black));
//                tracerArrowAmplitude->setPen(QPen(Qt::black, 4));
//                tracerArrowAmplitudeTop->setPen(QPen(Qt::black, 4));
//                tracerArrowAmplitudeBottom->setPen(QPen(Qt::black, 4));
            } else {
                tracer->setPen(QPen(Qt::white));
//                tracerArrowAmplitude->setPen(QPen(Qt::white, 4));
//                tracerArrowAmplitudeTop->setPen(QPen(Qt::white, 4));
//                tracerArrowAmplitudeBottom->setPen(QPen(Qt::white, 4));
            }
        }
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::on_scrollButton_clicked(bool checked) {
    if (checked) {
        ui->autoScrollLabel->setStyleSheet("QLabel { color : DeepPink; }");
        ui->autoScrollLabel->setText("Auto Scroll ON, To avoid autoscroll UNSELECT Button ---> ");
    } else {
        ui->autoScrollLabel->setStyleSheet("QLabel { color : DodgerBlue; }");
        ui->autoScrollLabel->setText("Auto Scroll OFF, To allow move cursor to the end or SELECT Button ---> ");
    }
}

/******************************************************************************************************************/
void MainWindow::on_logPlotButton_clicked() {
    if (logFile == nullptr) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Log Plot"),
                                   "",
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
        // Set Headers
        streamLog << "NAME" << ";" << "POINT" << ";" << "VALUE" << ";" << "TIME" << "\n";
        ui->logPlotButton->setText("Stop Logging");
    } else {
        qDebug() << "Log Plot Closed : " << logFile->fileName();
        ui->logPlotButton->setText("Log Displayed Plots");
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
}

/******************************************************************************************************************/
void MainWindow::contextMenuTriggered(QAction* act) {
    qDebug() << "contextMenuTriggered : " << act;
}
/******************************************************************************************************************/
void MainWindow::menuAboutToHide() {
    qDebug() << "menuAboutToHide";
}


void MainWindow::doContextMenuHeader(yaspGraph* yGraph) {
    contextMenu->setAttribute(Qt::WA_TranslucentBackground);
    contextMenu->setStyleSheet("QMenu {border-radius:16px;}");
    QWidget* menuTitleWidget = new QWidget();
    QLabel* menuTitleLabel = new QLabel("plot " + yGraph->plot()->name(), contextMenu);
    menuTitleLabel->setAlignment(Qt::AlignCenter);
    QLabel* menuIcon = new QLabel();
//    menuIcon->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png").scaledToWidth(200));
    menuIcon->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png"));
    QVBoxLayout* headerLayout = new QVBoxLayout;
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(menuTitleLabel);
    headerLayout->addWidget(menuIcon);
    menuTitleWidget->setLayout(headerLayout);
    //menuTitle->setAlignment(Qt::AlignCenter);
    QString styleTitle = "color:" + yGraph->plot()->pen().color().name() + ";"
//            + "background-color:grey;"
            + "background-color:" + bgColor.name() + ";"
            + "font:bold;"
            + "padding:8px;";
    //                + "border-radius:8px;";
    //        + "border-width:2px;border-style:solid;border-color:black;font:bold;padding:2px;";
    menuTitleWidget->setStyleSheet(styleTitle);
    QWidgetAction *LabelAction= new QWidgetAction(contextMenu);
    LabelAction->setDefaultWidget(menuTitleWidget);
    contextMenu->addAction(LabelAction);
}

///******************************************************************************************************************/
//void MainWindow::doContextMenuHeader(yaspGraph* yGraph) {
////    contextMenu->setTearOffEnabled(true);
//    contextMenu->setAttribute(Qt::WA_TranslucentBackground);
//    contextMenu->setStyleSheet("QMenu {border-radius:16px;}");
//    QLabel* menuTitle = new QLabel("plot " + yGraph->plot()->name(), contextMenu);
//////    menuTitle->setTextFormat(Qt::RichText);
//    menuTitle->setAlignment(Qt::AlignCenter);
//    QString styleTitle = "color:" + yGraph->plot()->pen().color().name() + ";"
//            + "background-color:" + bgColor.name() + ";"
//            + "font:bold;"
//            + "padding:8px;";
////                + "border-radius:8px;";
////        + "border-width:2px;border-style:solid;border-color:black;font:bold;padding:2px;";
//    menuTitle->setStyleSheet(styleTitle);

////    QLabel* menuTitle = new QLabel(contextMenu);
////    menuTitle->setText("<img src=\":/Icons/Icons/icons8-save-48.png\"> plot " + yGraph->plot()->name());
////    menuTitle->setAttribute(Qt::WA_TranslucentBackground);
////    menuTitle->setStyleSheet(styleTitle);
//    QWidgetAction *LabelAction= new QWidgetAction(contextMenu);
//    LabelAction->setDefaultWidget(menuTitle);
//    contextMenu->addAction(LabelAction);
//}

//void MainWindow::doContextMenuHeader(yaspGraph* yGraph) {
////    contextMenu->setTearOffEnabled(true);
//    contextMenu->setAttribute(Qt::WA_TranslucentBackground);
//    contextMenu->setStyleSheet("QMenu {border-radius:16px;}");
////    QLabel* menuTitle = new QLabel("plot " + yGraph->plot()->name(), contextMenu);
//    QLabel* menuTitle = new QLabel(contextMenu);
//    menuTitle->setTextFormat(Qt::RichText);
////    menuTitle->setText("<img src='qrc:/Icons/Icons/oscillo.ico'> plot " + yGraph->plot()->name());
//    menuTitle->setText("plot " + yGraph->plot()->name());

//    menuTitle->setAlignment(Qt::AlignCenter);
//    QString styleTitle = "color:" + yGraph->plot()->pen().color().name() + ";"
//            + "background-color:" + bgColor.name() + ";"
//            + "font:bold;"
//            + "padding:8px;";
////                + "border-radius:8px;";
////        + "border-width:2px;border-style:solid;border-color:black;font:bold;padding:2px;";
//    menuTitle->setStyleSheet(styleTitle);
//    QWidgetAction *LabelAction= new QWidgetAction(contextMenu);
//    LabelAction->setDefaultWidget(menuTitle);
//    contextMenu->addAction(LabelAction);
//    connect(LabelAction, SIGNAL(toggled(bool)), this, SLOT(doMenuCloseAction(bool)));
//}

/******************************************************************************************************************/
void MainWindow::plotLabelSelected(bool b) {
    qDebug() << "plotLabelSelected : " << b << "mouseState: " << mouseState << " workingGraph: " << workingGraph;
//    qDebug() << "ui->plot->selectedItems() size: " << ui->plot->selectedItems().size();
    if (b) {
        contextMenu->clear();
        qDebug() << "plotLabelSelected : " << ui->plot->selectedItems().size();
        if (ui->plot->selectedItems().size()) {
            Q_ASSERT(ui->plot->selectedItems().size() == 1);
            QCPAbstractItem* item = ui->plot->selectedItems().at(0);
            qDebug() << "plotLabelSelected : " << item << " property id " << item->property("id") << " / " << ui->plot->selectedItems().size();
            QVariant plotId = item->property("id");
            yaspGraph* yGraph = graphs[plotId.toInt()];
            Q_ASSERT(yGraph);
//            if (workingGraph) {
//                if (workingGraph != yGraph) {
//                    qDebug() << ">>>>>>>>>>>>>>>>> plotLabelSelected --> " << workingGraph << " / " << yGraph;
//                    workingGraph->setSelected(false);
//                } else {
//                    workingGraph->setSelected(false);
//    //                selectedPlotId = -1;
//    //                workingGraph = nullptr;
//    //                ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
//                    unselectGraphs();
//                    return;
//                }
//            }
            workingGraph = yGraph;
            contextMenu->setProperty("id", plotId);
            selectedPlotId = plotId.toInt();
        }
        Q_ASSERT(workingGraph);
        workingGraph->setSelected(true);
        doContextMenuHeader(workingGraph);
        if (mouseState != mouseDoMesure) {
            QAction* action = contextMenu->addAction("Color", this, SLOT(doMenuPlotColorAction()));
            action->setIcon(QIcon(":/Icons/Icons/icons8-paint-palette-48.png"));
            plotShowHideAction = contextMenu->addAction("Hide", this, SLOT(doMenuPlotShowHideAction()));
            if (workingGraph->plot()->visible()) {
                plotShowHideAction->setText("Hide");
                infoModeLabel->setText(workingGraph->plot()->name() + " --> SHOW MODE");
                infoModeLabel->setVisible(true);
                infoModeLabel->setColor(workingGraph->plot()->pen().color());
                plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-hide-48.png"));
            } else {
                plotShowHideAction->setText("Show");
                infoModeLabel->setText(workingGraph->plot()->name() + " --> HIDE MODE");
                infoModeLabel->setVisible(true);
                infoModeLabel->setColor(workingGraph->plot()->pen().color());
                plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
            }
//            QFontMetricsF fm(infoModeLabel->font());
//            qreal pixelsWide = fm.width(infoModeLabel->text());
//            infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
            infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
            action = contextMenu->addAction("Reset", this, SLOT(doMenuPlotResetAction()));
            action->setIcon(QIcon(":/Icons/Icons/icons8-available-updates-40.png"));
            action = contextMenu->addAction("Scale", this, SLOT(doMenuPlotScaleAction()));
            action->setIcon(QIcon(":/Icons/Icons/icons8-height-48.png"));
            action = contextMenu->addAction("Shift", this, SLOT(doMenuPlotShiftAction()));
            action->setIcon(QIcon(":/Icons/Icons/icons8-shift-48.png"));
            action = contextMenu->addAction("Save", this, SLOT(saveSelectedGraph()));
            action->setIcon(QIcon(":/Icons/Icons/icons8-save-48.png"));
            if (plotting == false) {
                plotMeasureAction = contextMenu->addAction("Start Measure", this, SLOT(doMenuPlotMeasureAction()));
                plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
            }
        } else {
            plotMeasureAction = contextMenu->addAction("Stop Measure", this, SLOT(doMenuPlotMeasureAction()));
            plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
        }
        ui->plot->setContextMenuPolicy(Qt::CustomContextMenu);
    } else {
//        if (workingGraph) {
//            qDebug() << "plotLabelSelected NOTHING !!!!";
//            workingGraph->setSelected(true);
//            return;
//        } else {
//            unselectGraphs();
//        }
//        selectedPlotId = -1;
//        workingGraph = nullptr;
//        ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
        unselectGraphs();
    }
}

/******************************************************************************************************************/
void MainWindow::xAxisRangeChanged(const QCPRange& range) {
    double val = range.upper - range.lower;
    if (!qFuzzyCompare(val, ui->spinDisplayTime->value())) {
        ui->spinDisplayTime->setValue(val);
    }
}

