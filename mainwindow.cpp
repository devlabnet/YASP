/***************************************************************************
**  This file is part of Yet Another Serial Plotter                       **
**                                                                        **
**  Serial Port Plotter is a program for plotting data from               **
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
**           Author:    Cricri042                                         **
**           Contact: cricri042@devlabnet.eu                              **
**           Date: 25/02/2019                                             **
****************************************************************************/

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QtGui>
#include <QPen>
#include <QNetworkReply>

//static const QString DEFS_URL = "https://www.devlabnet.eu/softdev/yasp/updates.json";
static const QString DEFS_URL = "https://raw.githubusercontent.com/devlabnet/YASP/master/installer/updates.json";
static const QString YASP_VERSION = "1.1.0";
static const QString DOC_URL = "https://gdoc.pub/doc/e/2PACX-1vQmyyZDie11-NvYd0V3Ry10cUGisbMw1lMT7EOq4qnecPBSdgyicpQix47Plv0QDT93KMiAFPEK7MNc";
/******************************************************************************************************************/
/* Constructor */
/******************************************************************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected(false), plotting(false), dataPointNumber(0), numberOfAxes(1),
    STATE(WAIT_START), plotTimeInMilliSeconds(PLOT_TIME_DEF) {
    ui->setupUi(this);
    QLocale::setDefault(QLocale::C);
    createUI();      // Create the UI
//    resize(minimumSize());
//    ui->terminalWidget->setVisible(false);
    // ----------------------------------------
    // Hide dynamicFrame and autoScrollCheckBox
    ui->dynamicFrame->setVisible(false);
    ui->autoScrollCheckBox->setVisible(false);
    // ----------------------------------------
    QColor gridColor = QColor(170,170,170);
    ui->bgColorButton->setAutoFillBackground(true);
    ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");
    QColor subGridColor = QColor(80,80,80);
    ui->plot->setBackground(QBrush(bgColor));                                    // Background for the plot area
    ui->plot->hide();
#ifdef YASP_PLOTS_WIDTH
    ui->plot->setOpenGl(true);
#endif
    ui->stopPlotButton->setEnabled(false);                                                // Plot button is disabled initially
    ui->logPlotButton->setVisible(false);
    // Legend
    ui->plot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->plot->setNotAntialiasedElements(QCP::aeAll);                                      // used for higher performance (see QCustomPlot real time example)
    // Y Axes
    ui->plot->yAxis->setTickLabelColor(gridColor);                              // See QCustomPlot examples / styled demo
    ui->plot->yAxis->setSelectedParts(QCPAxis::spNone);
    ui->plot->xAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->plot->xAxis->setSelectedParts(QCPAxis::spNone);
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
//    plotTimeInMilliSeconds = 30;
    ui->plot->xAxis->setTicker(fixedTicker);
    // tick step shall be 0.001 second -> 1 milisecond
    // tick step shall be 0.0001 second -> 0.1 milisecond -> 100 microdeconds
    // tick step shall be 0.00001 second -> 0.01 milisecond -> 10 microdeconds
    fixedTicker->setTickStep(0.00001);
    fixedTicker->setTickCount(10);
    fixedTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples );
    ui->plot->xAxis->setTickPen(QPen(Qt::red, 2));
    ui->plot->xAxis->setTickLength(15);
    // Slot for printing coordinates
    connect(ui->plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePressInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMoveInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseReleaseInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onMouseWheelInPlot(QWheelEvent*)));
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(xAxisRangeChanged(const QCPRange&)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(yAxisRangeChanged(const QCPRange&)));
    // setup policy and connect slot for context menu popup:
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(ui->plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(plotContextMenuRequest(QPoint)));
    ui->plot->setInteraction(QCP::iSelectAxes, false);
    ui->plot->setInteraction(QCP::iSelectPlottables, false);
    ui->plot->setInteraction(QCP::iSelectOther, false);

    connect(ui->checkBoxDynamicMeasures, SIGNAL(stateChanged(int)), this, SLOT(checkBoxDynamicMeasuresChanged(int)));
    serialPort = nullptr;
    // Connect update timer to replot slot
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(replot()));
    ui->menuWidgets->menuAction()->setVisible(false);
    updateTimer.setInterval(20);
    QPalette p;
    p.setColor(QPalette::Background, QColor(144, 238, 144));
    ui->splitter->setPalette(p);
    ui->splitter->setMinimumHeight(100);
//    ui->splitter->setSizes({2, 1});
    ui->splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));

    ui->tabWidget->setCurrentIndex(0);
    ui->spinDisplayTime->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->spinDisplayTime->setMinimum(PLOT_TIME_MIN_DEF);
    ui->spinDisplayTime->setMaximum(PLOT_TIME_MAX_DEF);
    ui->spinDisplayTime->setValue(PLOT_TIME_DEF);
    ui->spinDisplayTime->setDecimals(0);
    ui->spinDisplayTime->setSuffix(" Millis");
    ui->spinDisplayTime->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    ui->spinDisplayRange->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->spinDisplayRange->setMinimum(1);
    ui->spinDisplayRange->setMaximum(YAXIS_MAX_RANGE);
    ui->spinDisplayRange->setValue(DEF_YAXIS_RANGE);
    ui->spinDisplayRange->setDecimals(0);
    ui->spinDisplayRange->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    ui->autoScrollLabel->setStyleSheet("QLabel { color : DodgerBlue; }");
    ui->autoScrollLabel->setText("Auto Scroll OFF, To allow move cursor to the end or SELECT Button ---> ");
    ui->tabWidget->removeTab(1);
    contextMenu = new QMenu(this);
//    connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuTriggered(QAction*)));
//    connect(contextMenu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);    
    mouseWheelTimer.stop();

    ui->aboutNevVersionButton->setVisible(false);
    checkForUpdate();
//    loadHelpFile();
    // green rgb(153, 255, 153)
    // pink rgb(255, 179, 209)
    // yellow rgb(255, 255, 102)
    // blue rgb(102, 255, 255)
    ui->buttonsFrame->setStyleSheet("QFrame{color:black; background-color:rgb(153, 255, 153);border-radius:10px;border:2px solid grey} QLabel{border:none;}");
    ui->rangeFrame->setStyleSheet("QFrame{color:black; background-color:rgb(255, 179, 209);border-radius:10px;border:2px solid grey} QLabel{border:none;}");
    ui->dynamicFrame->setStyleSheet("QFrame{color:black; background-color:rgb(255, 255, 102);border-radius:10px;border:2px solid grey} QLabel{border:none;}");
    ui->pointsCountFrame->setStyleSheet("QFrame{color:black; background-color:rgb(102, 255, 255);border-radius:10px;border:2px solid grey} QLabel{border:none;}");
    measureMode = measureType::None;
    // Clear the terminal
    on_clearTermButton_clicked();
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
void MainWindow::checkForUpdate() {
    QNetworkRequest request;
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),   this, SLOT(checkForUpdateFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl(DEFS_URL)));
}

/******************************************************************************************************************/
/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 */
bool MainWindow::compareVersions(const QString& x, const QString& y) {
    QStringList versionsX = x.split (".");
    QStringList versionsY = y.split (".");
    int count = qMin (versionsX.count(), versionsY.count());
    for (int i = 0; i < count; ++i) {
        int a = QString (versionsX.at (i)).toInt();
        int b = QString (versionsY.at (i)).toInt();
        if (a > b) {
            return true;
        }
        if (b > a) {
            return false;
        }
    }
    return versionsY.count() < versionsX.count();
}

///******************************************************************************************************************/
//void MainWindow::loadHelpFile() {
//    ui->helpWebWidget->setUrl(QUrl(QStringLiteral("https://docs.google.com/document/d/e/2PACX-1vQmyyZDie11-NvYd0V3Ry10cUGisbMw1lMT7EOq4qnecPBSdgyicpQix47Plv0QDT93KMiAFPEK7MNc/pub")));
//    QFont ft = ui->helpWebWidget->font();
//    ui->helpWebWidget->setZoomFactor(1.5);
//}

/******************************************************************************************************************/
void MainWindow::setUpdateAvailable(bool available, QString latestVersion, QString changelog) {
    QString text;
    text += "<hr/>";
    text += "<div style=\"text-align:center;\">";
    text += "<img src=\":/Icons/Icons/logo_devlabnet_small.png\" alt=\"Smiley face\">";
    text += "</div>";
    text += "<hr/>";
    text += "<div style=\"text-align:center;color:black;font-size:20px;background-color:#ffffb3;\">";
    text += "<br>";
    text += tr ("%1 Version: %2").arg(qApp->applicationName()).arg(YASP_VERSION);
    text += "<br>";
    text += "</div>";

    if (available) {
        text += "<div style=\"text-align:center;color:black;font-size:20px;background-color:#ffb3b3;\">";
        text += "<br>";
        text += tr("Version %1 of %2 has been released!").arg (latestVersion).arg(qApp->applicationName());
        text += "<br><br>" + tr("If you like to download the update now");
        text += ",<br>";
        text += tr("Click on the \"Download New Version\" button above.");
        text += "<br>";
        text += "</div>";
        text += "<br>";
        text += "<hr/><b>New in version " + latestVersion + "</b>";
        text += changelog;
        text += "<hr/><br>";
        ui->AboutTextEdit->setHtml(text);
        ui->aboutNevVersionButton->setStyleSheet("color: Black; background-color: Tomato; font-weight:bold;");
        ui->aboutNevVersionButton->setVisible(true);
    } else  {
        text += "<hr/>";
        text += "<div style=\"text-align:center;color:black;font-size:20px;background-color:#d9ffb3;\">";
        text += "<br>";
        text += tr ("Congratulations!<br>You are running the latest version of %1").arg(qApp->applicationName());
        text += "<br>";
        text += tr("No updates are available for the moment");
        text += "<br>";
        text += "</div>";
        text += "<hr/><br>";
        ui->AboutTextEdit->setText(text);
        ui->aboutNevVersionButton->setVisible(false);
    }
}


/******************************************************************************************************************/
void MainWindow::checkForUpdateFinished(QNetworkReply* reply) {
    if(reply->error())  {
        qDebug() << "checkForUpdateFinished ERROR --> " << reply->errorString();
    }
    else
    {
//        qDebug() << __LINE__ << reply->header(QNetworkRequest::ContentTypeHeader).toString();
//        qDebug() << __LINE__ << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();;
//        qDebug() << __LINE__ << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
//        qDebug() << __LINE__ << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//        qDebug() << __LINE__ << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//        QString response =  reply->readAll();
//        qDebug() << "response: " << response;
        QJsonDocument document = QJsonDocument::fromJson (reply->readAll());
        /* JSON is invalid */
        if (document.isNull()) {
            qDebug() << "JSON is invalid !!!";
            downloadUrl = "";
            return;
        }
        QString platformKey = "windows";
        /* Get the platform information */
        QJsonObject updates = document.object().value ("updates").toObject();
        QJsonObject platform = updates.value (platformKey).toObject();
        /* Get update information */
        QString changelog = platform.value ("changelog").toString();
        downloadUrl = platform.value ("download-url").toString();
        QString latestVersion = platform.value ("latest-version").toString();
//        qDebug() << "changelog: " << changelog;
//        qDebug() << "downloadUrl: " << downloadUrl;
//        qDebug() << "latestVersion: " << latestVersion;
        setUpdateAvailable(compareVersions(latestVersion, YASP_VERSION), latestVersion, changelog);
    }
    reply->deleteLater();
}

/******************************************************************************************************************/
/**Create the GUI */
/******************************************************************************************************************/
void MainWindow::createUI() {
    if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
        enableControls(false);
        ui->connectButton->setEnabled(false);
        ui->statusBar->setStyleSheet("color: Black; background-color: Tomato; font-weight:bold;");
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
    ui->comboBaud->addItem("230400");
    ui->comboBaud->addItem("500000");
    ui->comboBaud->addItem("1000000");
    ui->comboBaud->addItem("2000000");
//    ui->comboBaud->addItem("2400000");
    ui->comboData->addItem("8 bits");                                                     // Populate data bits combo box
    ui->comboData->addItem("7 bits");
    ui->comboParity->addItem("none");                                                     // Populate parity combo box
    ui->comboParity->addItem("odd");
    ui->comboParity->addItem("even");
    ui->comboStop->addItem("1 bit");                                                      // Populate stop bits combo box
    ui->comboStop->addItem("2 bits");
    ui->comboBaud->setCurrentIndex(7);                                                    // Select 9600 bits by default
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
    ui->plot->clearPlottables();
//    ui->plot->hide();
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        delete yGraph;
    }
    graphs.clear();
    workingGraph = nullptr;
    measureMode = measureType::None;
}

/******************************************************************************************************************/
void MainWindow::cleanDataGraphsBefore(double d) {
//    d -= PLOT_TIME_MAX_DEF/2;
    if (d < PLOT_TIME_MAX_CLEAN_DEF) return;
    d -= PLOT_TIME_MAX_CLEAN_DEF;
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->plot()->data()->removeBefore(d);
    }
}

/******************************************************************************************************************/
void MainWindow::cleanDataGraphs() {
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->plot()->data()->clear();
    }
    dataPointNumber = 0;
}

/******************************************************************************************************************/
yaspGraph* MainWindow::addGraph(int id) {
        ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE/2, DEF_YAXIS_RANGE/2);       // Set lower and upper plot range
        QString plotStr = "Plot " + QString::number(id);
        QCPGraph* graph = ui->plot->addGraph();
        graph->setSelectable(QCP::stNone);
        QCPItemText* textLabel = new QCPItemText(ui->plot);
        textLabel->setSelectable(true);
        textLabel->setRoundCorners(5);
        textLabel->setPadding(QMargins(8, 4, 8, 4));
        textLabel->setProperty("id", id);
        connect(textLabel, SIGNAL(selectionChanged (bool)), this, SLOT(plotLabelSelectionChanged(bool)));
        QCPItemStraightLine* axisLine = new QCPItemStraightLine(ui->plot);
        axisLine->setSelectable(false);
        yaspGraph* g = new yaspGraph(id, graph, textLabel, axisLine, plotStr, colours[id], plotTimeInMilliSeconds);
        graphs.insert(id, g);
        return g;
}

/******************************************************************************************************************/
void MainWindow::updateLabel(int id, QString plotInfoStr) {
    yaspGraph* yGraph = graphs[id];
    Q_ASSERT(yGraph);
    yGraph->updateLabel(plotInfoStr, ui->plot->yAxis->axisRect()->left());
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
        ui->statusBar->setStyleSheet("background-color: Tomato; font-weight:bold;");
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
    ui->statusBar->setStyleSheet("background-color: SkyBlue ; font-weight:bold;");
    ui->statusBar->showMessage(selectedPort.description());

//    loadHelpFile();

}

/******************************************************************************************************************/
/* Connect Button clicked slot; handles connect and disconnect */
/******************************************************************************************************************/
void MainWindow::on_connectButton_clicked() {
    if (connected) {
        closePort();
    } else {
        openPort();
    }
}

/******************************************************************************************************************/
void MainWindow::initTracer() {
    // Plot points
    measureMode = measureType::None;
    points = new QCPGraph(ui->plot->xAxis, ui->plot->yAxis);
//    ui->plot->addPlottable(points);
    points->setSelectable(QCP::stNone);
    points->setAdaptiveSampling(false);
    points->removeFromLegend();
    points->setLineStyle(QCPGraph::lsNone);
    points->setScatterStyle(QCPScatterStyle::ssCircle);
    points->setPen(QPen(QBrush(Qt::red), 4));

    infoModeLabel = new QCPItemText(ui->plot);
    connect(infoModeLabel, SIGNAL(selectionChanged (bool)), this, SLOT(infoModeLabelSelectionChanged(bool)));
    infoModeLabel->setVisible(false);
    infoModeLabel->setSelectable(true);
    infoModeLabel->setRoundCorners(15);
    infoModeLabel->setBrush(QBrush(QColor(105,105,105)));
    infoModeLabel->setPadding(QMargins(8, 8, 8, 8));
    infoModeLabel->setSelectedColor(Qt::white);
    infoModeLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
    infoModeLabel->position->setType(QCPItemPosition::ptAbsolute );
    QFont font;
    font.setPointSize(12);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Bold);
    font.setStyle(QFont::StyleItalic);
    infoModeLabel->setFont(font);
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
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
    tracerRect->setSelectable(false);
    tracerRect->setPen(pen);
    tracerRect->setBrush(QBrush(QColor(200, 200, 200, 25)));
    pen.setStyle(Qt::DotLine);
    traceLineBottom = new QCPItemStraightLine(ui->plot);
    traceLineBottom->setSelectable(false);
    pen.setColor(Qt::green);
    traceLineBottom->setPen(pen);
    traceLineTop = new QCPItemStraightLine(ui->plot);
    traceLineTop->setSelectable(false);
    pen.setColor(Qt::red);
    traceLineTop->setPen(pen);
    traceLineLeft = new QCPItemStraightLine(ui->plot);
    traceLineLeft->setSelectable(false);
    pen.setColor(Qt::green);
    traceLineLeft->setPen(pen);
    traceLineRight = new QCPItemStraightLine(ui->plot);
    traceLineRight->setSelectable(false);
    pen.setColor(Qt::yellow);
    traceLineRight->setPen(pen);
    font.setPixelSize(12);
    tracerArrowAmplitude = new QCPItemLine(ui->plot);
    tracerArrowAmplitude->setSelectable(false);
    tracerArrowAmplitude->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitude->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitude->setPen(QPen(QColor(150, 255, 255), 4));
    tracerArrowAmplitudeTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeTxt->setSelectable(false);
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
    tracerArrowAmplitudeTop->setSelectable(false);
    tracerArrowAmplitudeTop->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeTop->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeTop->setPen(QPen(QColor(255, 150, 150), 4));
    tracerArrowAmplitudeTopTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeTopTxt->setSelectable(false);
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
    tracerArrowAmplitudeBottom->setSelectable(false);
    tracerArrowAmplitudeBottom->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeBottom->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowAmplitudeBottom->setPen(QPen(QColor(150, 255, 150), 4));
    tracerArrowAmplitudeBottomTxt = new QCPItemText(ui->plot);
    tracerArrowAmplitudeBottomTxt->setSelectable(false);
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
    tracerArrowFromRef->setSelectable(false);
    tracerArrowFromRef->setHead(QCPLineEnding::esSpikeArrow);
    tracerArrowFromRef->setTail(QCPLineEnding::esSpikeArrow);
    tracerArrowFromRef->setPen(QPen(QColor(255, 255, 150), 4));
    tracerArrowFromRefTxt = new QCPItemText(ui->plot);
    tracerArrowFromRefTxt->setSelectable(false);
    tracerArrowFromRefTxt->setRoundCorners(5);
    tracerArrowFromRefTxt->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
    tracerArrowFromRefTxt->setTextAlignment(Qt::AlignLeft);
    tracerArrowFromRefTxt->brush().setStyle(Qt::SolidPattern);
    tracerArrowFromRefTxt->setFont(font);
    tracerArrowFromRefTxt->setPen(QPen(bgColor));
    tracerArrowFromRefTxt->setColor(bgColor);
    tracerArrowFromRefTxt->setBrush(QBrush(QColor(255, 255, 150)));
    tracerArrowFromRefTxt->setPadding(QMargins(8, 4, 8, 4));
    resetTracer();
}

/******************************************************************************************************************/
/* Slot for port opened successfully */
/******************************************************************************************************************/
void MainWindow::portOpenedSuccess() {
    connect(serialPort, SIGNAL(dataTerminalReadyChanged(bool)), this, SLOT(dataTerminalReadyChanged(bool)));
    serialPort->setDataTerminalReady(false);
    ui->menuWidgets->menuAction()->setVisible(true);
    if ((widgetsW != nullptr)) {
        widgetsW->setSerialPort(serialPort);
    }
    ui->connectButton->setText("Disconnect");                                             // Change buttons
    ui->statusBar->setStyleSheet("background-color: SpringGreen ; font-weight:bold;");
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
//    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
//    connect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    QWidget* tabW = ui->tabWidget->findChild<QWidget *>("tabPlots");
    ui->tabWidget->insertTab(1, tabW, "Plots");
    ui->tabWidget->setCurrentIndex(1);
    updateTimer.start();
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInMilliSeconds, lastDataTtime);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    lastDataTtime = 0;
    connect(&mouseWheelTimer, SIGNAL(timeout()), this, SLOT(mouseWheelTimerShoot()));
    initTracer();
    on_resetPlotButton_clicked();
}

/******************************************************************************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape ) {
        if (workingGraph) {
            if (measureMode != measureType::None) {
                measureMode = measureType::None;
                resetTracer();
                plotLabelSelectionChanged(true);
                return;
            }
            unselectGraphs();
        }
    }
    if( event->modifiers() == Qt::ShiftModifier ) {
        if (measureMode == measureType::Arrow) {
            tracerArrowFromRef->setVisible(false);
            tracerArrowFromRefTxt->setVisible(false);
            tracerArrowAmplitudeTopTxt->setVisible(false);
            tracerArrowAmplitudeBottomTxt->setVisible(false);
        } else if (measureMode == measureType::Freq) {
            traceLineTop->setVisible(false);
            traceLineLeft->setVisible(false);
            traceLineRight->setVisible(false);
            tracerStep = 0;
            points->data()->clear();
            traceLineRight->setVisible(false);
            traceLineLeft->setVisible(false);
            traceLineTop->setVisible(false);
            tracerArrowAmplitudeTxt->setVisible(false);
        }
    }
    if( event->modifiers() == Qt::ControlModifier ) {
        if (measureMode == measureType::Arrow) {
            tracerArrowFromRef->setVisible(true);
            tracerArrowFromRefTxt->setVisible(true);
            tracerArrowAmplitudeTopTxt->setVisible(true);
            tracerArrowAmplitudeBottomTxt->setVisible(true);
            tracerArrowFromRef->start->setCoords(tracer->position->coords());
            tracerArrowFromRef->end->setCoords(tracer->position->coords());
        }
        else if (measureMode == measureType::Freq) {
            if (tracerStep == 0) {
                traceLineLeft->setVisible(true);
                traceLineTop->setVisible(false);
                traceLineRight->setVisible(true);
                double posX = ui->plot->xAxis->pixelToCoord(mousePos.x());
                traceLineLeft->point1->setCoords(posX, ui->plot->yAxis->range().lower);
                traceLineLeft->point2->setCoords(posX, ui->plot->yAxis->range().upper);
                traceLineRight->point1->setCoords(posX, -ui->plot->yAxis->range().lower);
                traceLineRight->point2->setCoords(posX, ui->plot->yAxis->range().upper);
                tracerStep = 1;
            } else if (tracerStep == 1) {
                tracerStep = 2;
                double lineY = (tracerRect->topLeft->coords().y() + tracerRect->bottomRight->coords().y()) / 2;
                traceLineTop->setVisible(true);
                traceLineTop->point1->setCoords(traceLineLeft->point1->coords().x(), lineY);
                traceLineTop->point2->setCoords(traceLineLeft->point1->coords().x(), lineY);
            }
        }
    }
}

/******************************************************************************************************************/
void MainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   if (workingGraph) {
       if (infoModeLabel && infoModeLabel->visible()) {
           infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
       }
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
    ui->plot->hide();
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
    if ((widgetsW != nullptr) && widgetsW->isVisible()) {
            widgetsW->hide();
    }
    updateTimer.stop();
    connected = false;
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
//    disconnect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
//    disconnect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    serialPort->close();                                                              // Close serial port
    delete serialPort;                                                                // Delete the pointer
    serialPort = nullptr;                                                                // Assign NULL to dangling pointer
    ui->connectButton->setText("Connect");                                            // Change Connect button text, to indicate disconnected
    ui->statusBar->setStyleSheet("background-color: SkyBlue; font-weight:bold;");
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
        if (plotting) {
            ui->plot->xAxis->setRange(lastDataTtime - plotTimeInMilliSeconds, lastDataTtime);
            ui->plot->replot();
        } else {
            ui->plot->xAxis->setRange(ui->plot->xAxis->range());
        }
    }
}

/******************************************************************************************************************/
void MainWindow::mouseWheelTimerShoot() {
    mouseWheelTimer.stop();
    if (workingGraph) {
        if (measureMode == measureType::Box) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        } else if (measureMode == measureType::Arrow) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE ARROW MODE");
        } else if (measureMode == measureType::Freq) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE FREQ MODE");
        } else {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> SHOW MODE");
        }
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    }
}

/******************************************************************************************************************/
void MainWindow::unselectGraphs() {
//    qDebug() << "<<<<<<<<<<<<< unselectGraphs >>>>>>>>>>>>>>";
    resetMouseWheelState();
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->setSelected(false);
    }
    selectedPlotId = -1;
    measureMode = measureType::None;
    resetTracer();
    workingGraph = nullptr;
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->plot->deselectAll();
    ui->plot->replot();
}

/******************************************************************************************************************/
/* Stop Plot Button */
/******************************************************************************************************************/
void MainWindow::on_stopPlotButton_clicked() {
    if (plotting) {
        // Stop plotting
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        // Stop updating plot timer
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
    } else {
        // Start plotting
        plotting = true;
        ui->stopPlotButton->setText("Stop Plot");
    }
    unselectGraphs();
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
    yaspGraph* graph = addGraph(id);
    return graph;
}

/******************************************************************************************************************/
void MainWindow::onNewPlotDataArrived(const QString& str) {
    QStringList newData = str.split(SPACE_MSG);               // Split string received from port and put it into list
    if (newData.size() > 1) {
        int plotId = newData.at(0).toInt();
        if ((plotId < 0) || (plotId > YASP_MAXPLOT_IND)) {
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
void MainWindow::onNewDataArrived(const QString &str) {
    QStringList newData = str.split(SPACE_MSG);               // Split string received from port and put it into list
    Q_ASSERT(newData.size() > 0);
    int plotId = newData.at(0).toInt();
    if ((plotId < 0) || (plotId > YASP_MAXPLOT_IND)) {
        addMessageText(" BAD DATA ID : " + QString::number(plotId) + " --> " + newData.join(" / "), "tomato");
        return;
    }
    yaspGraph* yGraph = getGraph(plotId);
    if (yGraph) {
        int dataListSize = newData.size();                                                    // Get size of received list
//        qDebug() << "NEW DATA : " << plotId << " / " << dataListSize << " --> " << newData;
        dataPointNumber++;
        if (dataListSize == 3) {
            double currentTime = newData[1].toDouble()/1000.0;
            if (currentTime < (lastDataTtime - YASP_OVERFLOW_TIME)) {
                // Will normally rarely append
                // Means that current millis() returned is more than YASP_OVERFLOW_TIME (1 Hour) in the past  !!
                // --> millis() overflow !! or Device Reset after more than 1 Hour logging (which lead to millis() / micros() reset to 0) !!
                // So just clean everything in graph
                qDebug() << currentTime <<  " ============================ CLEAN OVERFLOW ============================ " << lastDataTtime;
                cleanDataGraphs();
            }
            bool canReplot = ((currentTime - lastDataTtime) > 25);
            lastDataTtime = currentTime;
            cleanDataGraphsBefore(lastDataTtime);
            double val = newData[2].toDouble();
            // Add data to graphs according plot Id
            QCPGraph* plot = yGraph->plot();
            val *= yGraph->mult();
            val += yGraph->offset();
            yGraph->updateMinMax(val);
            plot->addData(lastDataTtime, val);
            ui->dataInfoLabel->setNum(dataPointNumber);
            QString plotInfoStr = " val: ";
            plotInfoStr += QString::number(val, 'f', 3);
            plotInfoStr +=  + " offset: ";
            plotInfoStr += QString::number(yGraph->offset(), 'f', 3);
            plotInfoStr +=  + " mult: ";
            plotInfoStr += QString::number(yGraph->mult(), 'f', 3);
            if (yGraph->getMin() < DBL_MAX) {
                plotInfoStr +=  + " min: ";
                plotInfoStr += QString::number(yGraph->getMin(), 'f', 3);
            }
            if (yGraph->getMax() > -DBL_MAX) {
                plotInfoStr +=  + " max: ";
                plotInfoStr += QString::number(yGraph->getMax(), 'f', 3);
            }
            if (ui->autoScrollCheckBox->isChecked() == false) {
                if (plotting) {
                    if (canReplot) {
                        replot();
                    }
                }
            }
            updateLabel(plotId, plotInfoStr);
        } else {
            qDebug() << "------------> BAD DATA : " << plotId << " / " << dataListSize << " --> " << str;
        }
    }
}

/******************************************************************************************************************/
void MainWindow::addMessageText(QString data, QString color) {
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
        qDebug() << "MISSED IN_MESSAGE !! State: " << STATE;
        QString recoverStr = data;
        int index = recoverStr.indexOf(START_MSG);
        QString msg = recoverStr.left(index);
        recoverStr = recoverStr.right(index);
        addMessageText(msg, "orange");
        addMessageText("-> DATA RECOVERED : " + recoverStr, "red");
        receivedData.clear();
        STATE = IN_MESSAGE;
        return true;
     }
     if ( cc == PLOT_MSG) {
        // Houps, seems we missed the END_MSG (maybe an external reset of the device !)
        // Just start PLOT_MSG scanning again
        qDebug() << "MISSED IN_PLOT_MSG !! State: " << STATE;
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
                        } else
                          if ( cc == '\n') {
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
                        onNewDataArrived(receivedData);
//                        QStringList incomingData = receivedData.split(SPACE_MSG);               // Split string received from port and put it into list
//                        emit newData(incomingData);                                       // Emit signal for data received with the list
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
//                        QStringList incomingData = receivedData.split(SPACE_MSG);               // Split string received from port and put it into list
//                        emit newPlotData(incomingData);
                        onNewPlotDataArrived(receivedData);
                        // Emit signal for data received with the list
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
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->reset();
    }
    plotTimeInMilliSeconds = PLOT_TIME_DEF;
    ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE/2, DEF_YAXIS_RANGE/2);       // Set lower and upper plot range
    ui->spinDisplayRange->setValue(ui->plot->yAxis->range().size());
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInMilliSeconds, lastDataTtime);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    ui->plot->replot();
}

/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinDisplayTime_valueChanged(double arg1) {
    plotTimeInMilliSeconds = arg1;
    if (plotting) {
        ui->plot->xAxis->setRange(lastDataTtime - plotTimeInMilliSeconds, lastDataTtime);
    } else {
        double oldRange = ui->plot->xAxis->range().size();
        double inc = (plotTimeInMilliSeconds - oldRange)/2.0;
        ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower - inc, ui->plot->xAxis->range().upper + inc);
    }
//    ui->plot->replot();
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
    Q_ASSERT(workingGraph);
    workingGraph->setMult(scale);
//    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::resetTracer() {
//    qDebug() << "========================== resetTracer ==========================";
    points->setVisible(false);
    points->data()->clear();
    ui->plot->setCursor(Qt::ArrowCursor);
    tracer->setVisible(false);
    tracerStep = 0;
    togglePlotsVisibility(true);
    infoModeLabel->setVisible(false);
    traceLineTop->point1->setCoords(ui->plot->xAxis->range().lower, ui->plot->yAxis->range().lower);
    traceLineTop->point2->setCoords(ui->plot->xAxis->range().upper, ui->plot->yAxis->range().lower);
    traceLineBottom->point1->setCoords(ui->plot->xAxis->range().lower, ui->plot->yAxis->range().upper);
    traceLineBottom->point2->setCoords(ui->plot->xAxis->range().upper, ui->plot->yAxis->range().upper);
    traceLineBottom->setVisible(false);
    traceLineTop->setVisible(false);
    traceLineLeft->setVisible(false);
    traceLineRight->setVisible(false);
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
      ui->plot->removeItem(*lI);
    }
    tracerHLinesRef.clear();
    lI = tracerHLinesTracer.end();
    while(lI != tracerHLinesTracer.begin()) {
      --lI;
      ui->plot->removeItem(*lI);
    }
    tracerHLinesTracer.clear();
    QList<QCPItemText*>::iterator tI = tracerHLinesRefInfo.end();
    while(tI != tracerHLinesRefInfo.begin()) {
      --tI;
      ui->plot->removeItem(*tI);
    }
    tracerHLinesRefInfo.clear();
    tI = tracerHLinesTracerInfo.end();
    while(tI != tracerHLinesTracerInfo.begin()) {
      --tI;
      ui->plot->removeItem(*tI);
    }
    tracerHLinesTracerInfo.clear();
    measureMode = measureType::None;
    resetMouseWheelState();
}

/******************************************************************************************************************/
void MainWindow::togglePlotsVisibility(bool show) {
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        yGraph->toggleVisibility(show);
    }
}

/******************************************************************************************************************/
void MainWindow::ShowPlotsExceptWG(bool show) {
    if (workingGraph == nullptr) return;
    foreach (yaspGraph* yGraph, graphs) {
        Q_ASSERT(yGraph);
        if (workingGraph == yGraph) continue;
        yGraph->toggleVisibility(show);
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotMeasureAction() {
//    qDebug() << "doMenuPlotMeasureAction: " << contextMenu->property("id") << " measureInProgress: " << measureMode;
    Q_ASSERT(contextMenu);
    Q_ASSERT(workingGraph);
    if (measureMode != measureType::None) {
        resetTracer();
        plotLabelSelectionChanged(true);
    } else {
        measureMode = measureType::Arrow;
        ShowPlotsExceptWG(false);
        measureMult = workingGraph->mult();
        infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        plotLabelSelectionChanged(true);
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
        ui->plot->setCursor(Qt::CrossCursor);
        tracer->setVisible(true);
        tracer->setGraph(workingGraph->plot());
        updateTracerMeasure();
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotMeasureFreqAction() {
//   qDebug() << "doMenuPlotMeasureFreqAction: " << contextMenu->property("id");
   Q_ASSERT(contextMenu);
   Q_ASSERT(workingGraph);
   if (measureMode != measureType::None) {
       resetTracer();
       plotLabelSelectionChanged(true);
   } else {
       measureMode = measureType::Freq;
       ShowPlotsExceptWG(false);
       measureMult = workingGraph->mult();
       infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE FREQ MODE");
       infoModeLabel->setVisible(true);
       infoModeLabel->setColor(workingGraph->plot()->pen().color());
       plotLabelSelectionChanged(true);
       infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
       ui->plot->setCursor(Qt::CrossCursor);
       updateTracerFrequency();
   }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotMeasureBoxAction() {
//    qDebug() << "doMenuPlotMeasureBoxAction: " << contextMenu->property("id") << " measureInProgress: " << measureInProgress;
    Q_ASSERT(contextMenu);
    Q_ASSERT(workingGraph);
    if (measureMode != measureType::None) {
        resetTracer();
        plotLabelSelectionChanged(true);
    } else {
        measureMode = measureType::Box;
        ShowPlotsExceptWG(false);
        measureMult = workingGraph->mult();
        infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        plotLabelSelectionChanged(true);
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
        ui->plot->setCursor(Qt::CrossCursor);
        tracer->setVisible(true);
        tracer->setGraph(workingGraph->plot());
        traceLineTop->point1->setCoords(ui->plot->xAxis->range().lower, ui->plot->yAxis->range().lower);
        traceLineTop->point2->setCoords(ui->plot->xAxis->range().upper, ui->plot->yAxis->range().lower);
        traceLineBottom->point1->setCoords(ui->plot->xAxis->range().lower, ui->plot->yAxis->range().upper);
        traceLineBottom->point2->setCoords(ui->plot->xAxis->range().upper, ui->plot->yAxis->range().upper);
        updateTracerBox();
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotColorAction() {
//    qDebug() << "doMenuPlotColorAction: " << contextMenu->property("id");
    resetMouseWheelState();
    Q_ASSERT(workingGraph);
    infoModeLabel->setText(workingGraph->plot()->name() + " --> COLOR MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(workingGraph->plot()->pen().color());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    QColor color = QColorDialog::getColor(Qt::white, nullptr, "plot color");
    if (color.isValid()) {
        QPen pen = workingGraph->plot()->pen();
        pen.setColor(color);
        workingGraph->plot()->setPen(pen);
        workingGraph->setSelected(true);
    }
    resetMouseWheelState();
    infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");
}


#ifdef YASP_PLOTS_WIDTH
/******************************************************************************************************************/
void MainWindow::doMenuPlotWidthAction(int w) {
    qDebug() << "doMenuPlotWidthAction: " << contextMenu->property("id");
    qDebug() << "doMenuPlotWidthAction width: " << w;
    Q_ASSERT(workingGraph);
    QPen pen = workingGraph->plot()->pen();
    pen.setWidth(w);
    workingGraph->plot()->setPen(pen);
}
#endif

/******************************************************************************************************************/
void MainWindow::doMenuPlotResetAction() {
//    doMenuPlotMeasureBoxAction();
//    doMenuPlotMeasureAction();
//    return;
//    qDebug() << "doMenuPlotResetAction: " << contextMenu->property("id");
    resetMouseWheelState();
    Q_ASSERT(workingGraph);
    workingGraph->reset();
    ui->plot->replot();
    infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotShowHideAction() {
//    qDebug() << "doMenuPlotShowHideAction: " << workingGraph->plot()->name();
    resetMouseWheelState();
    Q_ASSERT(workingGraph);
    if (workingGraph->plot()->visible()) {
        workingGraph->hide(true);
        plotShowHideAction->setText("show");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
        infoModeLabel->setText(workingGraph->plot()->name() + " --> HIDE MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
    } else {
        workingGraph->hide(false);
        plotShowHideAction->setText("Hide");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-hide-48.png"));
        infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
    }
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    plotLabelSelectionChanged(true);
    ui->plot->replot();
}

//*******************************************************************************************/
void MainWindow::saveDataPlot() {
    Q_ASSERT(workingGraph);
    if (!workingGraph->plot()->visible()) return;
    infoModeLabel->setText(workingGraph->plot()->name() + " --> SAVE MODE");
    infoModeLabel->setVisible(true);
    infoModeLabel->setColor(workingGraph->plot()->pen().color());
    QString plotName = workingGraph->plot()->name();
    if (logData == nullptr) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Log Plot"),
                                   plotName,
                                   tr("Data (*.csv)"));
//        qDebug() << "Log DATA : " << fileName;
        logData = new QFile(fileName);
        if (!logData->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     logData->errorString());
            logData = nullptr;
            return;
        }
//        qDebug() << "Log DATA Opened : " << logData->fileName();
        streamData.setDevice(logData);
        workingGraph->save(streamData);
    }
//   qDebug() << "Close Log DATA : " << logData->fileName();
   logData->close();
   delete logData;
   logData = nullptr;
   infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");

}

/******************************************************************************************************************/
void MainWindow::saveSelectedGraph() {
//    qDebug() << "saveSelectedGraph: " << contextMenu->property("id");
    Q_ASSERT(selectedPlotId >= 0);
    workingGraph = graphs[selectedPlotId];
    Q_ASSERT(workingGraph);
    saveDataPlot();
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
void MainWindow::updateTracerMeasure() {
    Q_ASSERT(workingGraph);
    if (measureMode == measureType::Arrow) {
        Q_ASSERT(tracer);
        double pX = mousePos.x();
        QList<QCPItemLine*>::iterator lI;
        lI = tracerHLinesRef.end();
        while(lI != tracerHLinesRef.begin()) {
            --lI;
            ui->plot->removeItem(*lI);
        }
        tracerHLinesRef.clear();
        QList<QCPItemText*>::iterator tI = tracerHLinesRefInfo.end();
        while(tI != tracerHLinesRefInfo.begin()) {
            --tI;
            ui->plot->removeItem(*tI);
        }
        tracerHLinesRefInfo.clear();
        double coordX = ui->plot->xAxis->pixelToCoord(pX);
        // get tracer origin
        tracer->setGraphKey(tracerStartKey);
        tracer->updatePosition();
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
        double startX = tracerArrowFromRef->start->coords().x();
        double startY = tracerArrowFromRef->start->coords().y();
        double mult = workingGraph->mult();
        double endX = tracer->position->key();
        double endY = tracer->position->value();
        double plotWidth = ui->plot->xAxis->range().size();
        double space = plotWidth / 50;
        tracerArrowFromRef->start->setCoords(startX, startY);
        tracerArrowFromRef->end->setCoords(tracer->position->coords());
        QLineF tracerArrowLine = QLineF(startX, startY, endX, endY);
        tracerArrowAmplitudeTopTxt->setFont(QFont(font().family(), 8));
        tracerArrowAmplitudeBottomTxt->setFont(QFont(font().family(), 8));
        tracerArrowFromRefTxt->setFont(QFont(font().family(), 8));
        tracerArrowFromRefTxt->setPositionAlignment(Qt::AlignCenter|Qt::AlignVCenter);
        tracerArrowFromRefTxt->setTextAlignment(Qt::AlignCenter);
        tracerArrowFromRefTxt->position->setCoords(tracerArrowLine.center());
        if (startX > endX) {
            tracerArrowAmplitudeBottomTxt->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            tracerArrowAmplitudeBottomTxt->setTextAlignment(Qt::AlignRight);
            tracerArrowAmplitudeBottomTxt->position->setCoords(startX + space, startY);
            tracerArrowAmplitudeTopTxt->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
            tracerArrowAmplitudeTopTxt->setTextAlignment(Qt::AlignLeft);
            tracerArrowAmplitudeTopTxt->position->setCoords(endX - space, endY);
        } else {
            tracerArrowAmplitudeBottomTxt->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
            tracerArrowAmplitudeBottomTxt->setTextAlignment(Qt::AlignLeft);
            tracerArrowAmplitudeBottomTxt->position->setCoords(startX - space, startY);
            tracerArrowAmplitudeTopTxt->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            tracerArrowAmplitudeTopTxt->setTextAlignment(Qt::AlignRight);
            tracerArrowAmplitudeTopTxt->position->setCoords(endX + space, endY);
        }
        QString str = QString::number(startX, 'f', 3) + " / " + QString::number(startY, 'f', 3);
        if (!qFuzzyCompare(mult, 1.0)) {
            str += " [" + QString::number(startY/mult, 'f', 3) + "]";
        }
        tracerArrowAmplitudeBottomTxt->setText(str);
        str = QString::number(endX, 'f', 3) + " / " + QString::number(endY, 'f', 3);
        if (!qFuzzyCompare(mult, 1.0)) {
            str += " [" + QString::number(endY/mult, 'f', 3) + "]";
        }
        tracerArrowAmplitudeTopTxt->setText(str);
        str = QString::number(tracerArrowLine.dx(), 'f', 3) + " / " + QString::number(tracerArrowLine.dy(), 'f', 3);
        if (!qFuzzyCompare(mult, 1.0)) {
            str += " [" + QString::number(tracerArrowLine.dy()/mult, 'f', 3) + "]";
        }
        tracerArrowFromRefTxt->setText(str);
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::insertIntersectionPoint(QLineF ref, double x0, double y0, double x1, double y1) {
    QLineF line0 = QLineF(x0, y0, x1, y1);
//    qDebug() << "== " << ref << "y0: " << y0 << "x0: " << x0 << " y1: " << y1 << "x1: " << x1 << " line " << line0;
//    qDebug() << "-> " << ref.intersect(line0, &intersectPoint) << " point: " << intersectPoint;
    if (ref.intersect(line0, &intersectPoint) == QLineF::BoundedIntersection) {
        points->addData(intersectPoint.x(), intersectPoint.y());
    }
}

/******************************************************************************************************************/
void MainWindow::updateTracerFrequency() {
    Q_ASSERT(workingGraph);
    if (measureMode == measureType::Freq) {
        //    double posX = ui->plot->yAxis->pixelToCoord(mousePos.x());
        double posY = ui->plot->yAxis->pixelToCoord(mousePos.y());
        if (tracerStep == 1) {
            // Create / Adjust Vertical Lines
            double posX = ui->plot->xAxis->pixelToCoord(mousePos.x());
            double right = posX;
            traceLineRight->point1->setCoords(right, ui->plot->yAxis->range().lower);
            traceLineRight->point2->setCoords(right, ui->plot->yAxis->range().upper);
        }
        if (tracerStep == 2) {
            double left = traceLineLeft->point1->coords().x();
            double right =  traceLineRight->point1->coords().x();
            traceLineTop->point1->setCoords(left, posY);
            traceLineTop->point2->setCoords(right, posY);
            // Measures
            QSharedPointer<QCPGraphDataContainer> gData = workingGraph->plot()->data();
            QCPDataContainer<QCPGraphData>::const_iterator itStart;
            QCPDataContainer<QCPGraphData>::const_iterator itEnd;
            bool rangeOk = false;
            if (right > left) {
                itStart = gData->findBegin(left, false);
                itEnd = gData->findBegin(right, true);
                if (itEnd->key > itStart->key ) {
                    rangeOk = true;
                }
            } else {
                itStart = gData->findBegin(right, false);
                itEnd = gData->findBegin(left, true);
                if (itEnd->key > itStart->key ) {
                    rangeOk = true;
                }
            }
            if (rangeOk) {
                double minVal = DBL_MAX;
                double maxVal = -DBL_MAX;
                //            qDebug() << "================================";
                //            qDebug() << itStart->key << " / " << itEnd->key;
                //            qDebug() << "====  ====  ====  ====  ====";
                double x0 = itStart->key;
                double x1 = itStart->key;;
                double y0 = itStart->value;
                double y1 = itStart->value;
                points->setVisible(true);
                //             qDebug() << "clear points !";
                points->data()->clear();

                for (QCPDataContainer<QCPGraphData>::const_iterator it = itStart; it <= itEnd; ++it) {
                    //                xVal = it->key;
                    //                yVal = it->value;
                    x0 = it->key;;
                    y0 = it->value;
                    if (qFuzzyCompare(y0, posY)) {
                        insertIntersectionPoint(QLineF(traceLineLeft->point1->coords().x(), posY,
                                                       traceLineRight->point1->coords().x(), posY),
                                                x0, y0, x1, y1);
                    } else if ((y0 < posY) && (y1 > posY)) {
                        insertIntersectionPoint(QLineF(traceLineLeft->point1->coords().x(), posY,
                                                       traceLineRight->point1->coords().x(), posY),
                                                x0, y0, x1, y1);
                    } else if ((y0 > posY) && (y1 < posY)) {
                        insertIntersectionPoint(QLineF(traceLineLeft->point1->coords().x(), posY,
                                                       traceLineRight->point1->coords().x(), posY),
                                                x0, y0, x1, y1);
                    }
                    if (y0 > maxVal) {
                        maxVal = y0;
                    }
                    if (y0 < minVal) {
                        minVal = y0;
                    }
                    x1 = x0;
                    y1 = y0;
                }
                tracerArrowAmplitudeTxt->setFont(QFont(font().family(), 8));
                tracerArrowAmplitudeTxt->setVisible(true);
                QString infoStr;
                tracerArrowAmplitudeTxt->position->setCoords(traceLineLeft->point1->coords().x() - 30, posY);
                if (qFuzzyCompare(workingGraph->mult(), 1.0)) {
                    infoStr = "Amplitude: " + QString::number(maxVal - minVal, 'f', 3);
                } else {
                    infoStr = "Amplitude: " + QString::number(maxVal - minVal, 'f', 3)
                            + " [" + QString::number((maxVal - minVal)/workingGraph->mult(), 'f', 3) + "] ";
                }
                //            qDebug() << "============================";
                //            qDebug() << "Min: " << minVal << " / Max: " << maxVal;
                //            qDebug() << "====  ====  ====  ====  ====";
                double periods = 0;
                double period;
                if (points->dataCount() > 2) {
                    QCPDataContainer<QCPGraphData>::const_iterator it = points->data()->constBegin();
                    int cnt = points->dataCount() / 3;
                    //                qDebug() << "CNT -> " << cnt;
                    for (int i=0; i < cnt; ++i) {
                        double v0 = it->key;
                        it++;
                        it++;
                        double v2 = it->key;
                        period = v2 - v0;
                        periods += period;
                    }
                    infoStr += "\nPeriod : ";
                    period = periods/cnt/1000.0;   // in Seconds
                    infoStr += QString::number(period, 'f', 6);
                    infoStr += " Sec\nFrequency : ";
                    infoStr += QString::number(1.0/(period), 'f', 6);
                    infoStr += " Hz";
                }
                tracerArrowAmplitudeTxt->setText(infoStr);
            }
        }
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::updateTracerBox() {
    Q_ASSERT(workingGraph);
    if (measureMode == measureType::Box) {
        Q_ASSERT(tracer);
        double pX = mousePos.x();
        QList<QCPItemLine*>::iterator lI;
        lI = tracerHLinesRef.end();
        while(lI != tracerHLinesRef.begin()) {
          --lI;
          ui->plot->removeItem(*lI);
        }
        tracerHLinesRef.clear();
        lI = tracerHLinesTracer.end();
        while(lI != tracerHLinesTracer.begin()) {
          --lI;
          ui->plot->removeItem(*lI);
        }
        tracerHLinesTracer.clear();
        QList<QCPItemText*>::iterator tI = tracerHLinesRefInfo.end();
        while(tI != tracerHLinesRefInfo.begin()) {
          --tI;
          ui->plot->removeItem(*tI);
        }
        tracerHLinesRefInfo.clear();
        tI = tracerHLinesTracerInfo.end();
        while(tI != tracerHLinesTracerInfo.begin()) {
          --tI;
          ui->plot->removeItem(*tI);
        }
        tracerHLinesTracerInfo.clear();
        double plotWidth = ui->plot->xAxis->range().size();
        double plotHeight = ui->plot->yAxis->range().size();
        double ref = workingGraph->offset();
        double space = plotWidth / 800;
        double coordX = ui->plot->xAxis->pixelToCoord(pX);
        // get tracer origin
        tracer->setGraphKey(tracerStartKey);
        tracer->updatePosition();
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
        double endX = tracer->position->key();
        double endY = tracer->position->value();
        traceLineBottom->setVisible(true);
        traceLineTop->setVisible(true);
        tracerRect->setVisible(true);
        double gMin = workingGraph->getMin();
        double gMax = workingGraph->getMax();
        traceLineBottom->point1->setCoords(ui->plot->xAxis->range().lower, gMin);
        traceLineBottom->point2->setCoords(ui->plot->xAxis->range().upper, gMin);
        traceLineTop->point1->setCoords(ui->plot->xAxis->range().lower, gMax);
        traceLineTop->point2->setCoords(ui->plot->xAxis->range().upper, gMax);
//        double plotVCenter = (traceLineTop->point1->coords().y() + traceLineBottom->point1->coords().y()) / 2.0;
//        double rh = plotHeight * 0.35;
//        double tracerRectTop = plotVCenter + rh;
//        double tracerRectBottom = plotVCenter - rh;
        double tracerRectTop =gMax + plotHeight/10;
        double tracerRectBottom = gMin - plotHeight/10;
        double tracerRectLeft = endX - (plotWidth/3);
        double tracerRectRight = endX + (plotWidth/3);
        tracerRect->topLeft->setCoords(tracerRectLeft, tracerRectTop);
        tracerRect->bottomRight->setCoords(tracerRectRight, tracerRectBottom);
        // Vertical measures
        double amplitude = traceLineTop->point1->coords().y() - traceLineBottom->point1->coords().y();
        double deltaTop = traceLineTop->point1->coords().y() - endY;
        double deltaBottom = endY - traceLineBottom->point1->coords().y();
        double deltaFromRef = endY - ref;
        double space0 = 15.0;
        double space1 = 30.0;
        QString tracerInfo("Ref: %1 Delta+: %2 Delta-: %3 Min: %4 Max: %5 Amplitude: %6 Raw Amplitude: %7");
        tracerInfo =  tracerInfo.arg(endY)
                .arg(deltaTop)
                .arg(deltaBottom)
                .arg(traceLineBottom->point1->coords().y())
                .arg(traceLineTop->point1->coords().y())
                .arg(amplitude)
                .arg(amplitude / workingGraph->mult());
        ui->statusBar->setStyleSheet("background-color: lightgreen; font-weight:bold;");
        ui->statusBar->showMessage(tracerInfo);
        double ampTracerArrowSY = traceLineBottom->point1->coords().y();
        double ampTracerArrowEY = traceLineTop->point2->coords().y();
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
                tracerArrowAmplitudeTxt->setText(QString::number(amplitude, 'f', 3));
            } else {
                tracerArrowAmplitudeTxt->setText(QString::number(amplitude, 'f', 3) + " ["
                                                 + QString::number(amplitude/mult, 'f', 3) + "]");
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
                tracerArrowAmplitudeTopTxt->setText(QString::number(deltaTop, 'f', 3));
            } else {
                tracerArrowAmplitudeTopTxt->setText(QString::number(deltaTop, 'f', 3) + " ["
                                                 + QString::number(deltaTop/mult, 'f', 3) + "]");
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
                tracerArrowAmplitudeBottomTxt->setText(QString::number(deltaBottom, 'f', 3));
            } else {
                tracerArrowAmplitudeBottomTxt->setText(QString::number(deltaBottom, 'f', 3) + " ["
                                                 + QString::number(deltaBottom/mult, 'f', 3) + "]");
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
                tracerArrowFromRefTxt->setText(QString::number(deltaFromRef, 'f', 3));
            } else {
                tracerArrowFromRefTxt->setText(QString::number(deltaFromRef, 'f', 3) + " ["
                                                 + QString::number(deltaFromRef/mult, 'f', 3) + "]");
            }
            if (deltaFromRef > 0) {
                tracerArrowFromRefTxt->position->setCoords(coordX - (space0+space1)*space,
                                                             (ref + endY)/2.0 + 150);
            } else {
                tracerArrowFromRefTxt->position->setCoords(coordX - (space0+space1)*space,
                                                             (ref + endY)/2.0 - 150);
            }
        }
        // Horizontal measures
        QSharedPointer<QCPGraphDataContainer> gData = workingGraph->plot()->data();
        QCPDataContainer<QCPGraphData>::const_iterator itStart = gData->findBegin(tracerRectLeft, true);
        QCPDataContainer<QCPGraphData>::const_iterator itEnd = gData->findBegin(tracerRectRight, true);
        lastTracerXValueRef = itStart->key;
        lastTracerXValueTracer = itStart->key;
        double deltaMinTracer = 0;
        double deltaMinRef = 0;
        double lastItValue = itStart->value;
        bool valueChanged = false;
        for (QCPDataContainer<QCPGraphData>::const_iterator it = itStart; it != itEnd; ++it) {
            valueChanged = !qFuzzyCompare(lastItValue, it->value);
            if (valueChanged) {
                double deltaFromRef = abs(ref - (it->value))/mult;
//                qDebug() << it->value << " previous " << lastItValue << " changed " << valueChanged;
                if (deltaFromRef < 1) {
                    double deltaRef = it->key - lastTracerXValueRef;
                    if (deltaRef > deltaMinRef) {
                        deltaMinRef = space * 20;
                        QCPItemLine* line = new QCPItemLine(ui->plot);
                        line->setSelectable(false);
                        tracerHLinesRef.append(line);
                        // Orange
                        //                    QColor col(255,165,0);
                        QColor col(255,140,0);
                        QPen pen(col, 2);
                        pen.setStyle(Qt::DotLine);
                        line->setPen(pen);
                        line->start->setCoords(it->key, it->value);
                        line->end->setCoords(it->key, tracerRectBottom);
                        if (tracerHLinesRef.size() > 1) {
                            // Add Time info between lines
                            QCPItemText* info = new QCPItemText(ui->plot);
                            info->setSelectable(false);
                            info->setText(QString::number(deltaRef, 'f', 3));
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
                double deltaFromTracer = abs(it->value - endY)/mult;
                if (deltaFromTracer < 1) {
                    double deltaTracer = it->key - lastTracerXValueTracer;
                    if (deltaTracer > deltaMinTracer) {
                        deltaMinTracer = space * 20;
                        QCPItemLine* line = new QCPItemLine(ui->plot);
                        line->setSelectable(false);
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
                            info->setSelectable(false);
                            info->setText(QString::number(deltaTracer, 'f', 3));
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
                lastItValue = it->value;
            }
        }
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseMoveInPlot(QMouseEvent *event) {
    //    qDebug() << "onMouseMoveInPlot " << measureInProgress;
    mousePos =event->pos();
    double xx = ui->plot->xAxis->pixelToCoord(event->x());
    double yy = ui->plot->yAxis->pixelToCoord(event->y());
    QString coordinates(" -- Time: %1 Seconds -- Y: %2 -- ");
    coordinates = coordinates.arg(xx/1000.0,0,'f',3).arg(yy,0,'f',3);
    ui->statusBar->setStyleSheet("background-color: SkyBlue; font-weight:bold;");
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    if (workingGraph) {
        if (measureMode == measureType::None) {
            if (event->buttons() == Qt::LeftButton) {
                // Just shift the selected plot
                ui->plot->setInteractions(QCP::iRangeZoom | QCP::iSelectItems );
                infoModeLabel->setText(workingGraph->plot()->name() + " -> SHIFT MODE");
                infoModeLabel->setColor(workingGraph->plot()->pen().color());
                infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
                shiftPlot(event->y());
                QString msg = "Moving PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->offset(), 'f', 3);
                ui->statusBar->showMessage(msg);
            }
        } else {
            if (event->buttons() != Qt::LeftButton) {
                if (measureMode == measureType::Box) {
                    updateTracerBox();
                } else if (measureMode == measureType::Arrow) {
                    updateTracerMeasure();
                } else if (measureMode == measureType::Freq) {
                    updateTracerFrequency();
                }
            }
        }
        ui->statusBar->showMessage(coordinates);
    } else { // no workingGraph
        ui->statusBar->showMessage(coordinates);
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event) {
    Q_UNUSED(event)
    if (workingGraph) {
        startShiftPlot = false;
        if (measureMode == measureType::Box) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        } else if (measureMode == measureType::Arrow) {
             infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE ARROW MODE");
        } else if (measureMode == measureType::Freq) {
             infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE FREQ MODE");
        } else {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> SHOW MODE");
        }
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    } else {
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    }
}

/******************************************************************************************************************/
void MainWindow::resetMouseWheelState() {
    mouseWheelTimer.stop();
    startShiftPlot = false;
//    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::onMouseWheelInPlot(QWheelEvent *event) {
    QPoint mPos = event->pos();
    if (event->buttons() == Qt::RightButton) {
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
        QPoint numDegrees = event->angleDelta();
        if (numDegrees.y() == 0) return;
        double inc = plotTimeInMilliSeconds / 100.0;

        if (numDegrees.y() > 0) {
            if (plotTimeInMilliSeconds > PLOT_TIME_MAX_DEF) {
                return;
            }
            plotTimeInMilliSeconds += inc;
        } else {
            if (plotTimeInMilliSeconds < PLOT_TIME_MIN_DEF) {
                return;
            }
            plotTimeInMilliSeconds -= inc;
        }
        if (plotting) {
            ui->plot->xAxis->setRange(lastDataTtime - plotTimeInMilliSeconds, lastDataTtime);
        } else {
            if (numDegrees.y() > 0) {
                ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower - inc, ui->plot->xAxis->range().upper + inc);
            } else {
                ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower + inc, ui->plot->xAxis->range().upper - inc);
            }
        }
    } else {
        // zoom display
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
          if (workingGraph && (measureMode == measureType::None)) {
                ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
                QPoint numDegrees = event->angleDelta();
                int nY = numDegrees.y();
                if (nY != 0) {
                    mouseWheelTimer.start(500);
                    infoModeLabel->setText(workingGraph->plot()->name() + " -> SCALE MODE");
                    double delta = static_cast<double>(nY) / 1000.0;
                    double scale = 1 + delta;
                    scalePlot(scale);
                    double p0 = ui->plot->yAxis->pixelToCoord(mPos.y());
                    double p1 = (p0 - workingGraph->offset()) * delta;
                    ui->plot->yAxis->setRange(ui->plot->yAxis->range().lower + p1, ui->plot->yAxis->range().upper + p1);
                    QString msg = "Scaling PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->mult(), 'f', 3);
                    ui->statusBar->showMessage(msg);
                }
        } else {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
        }
    }
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::onMousePressInPlot(QMouseEvent *event) {
    Q_UNUSED(event);

    if (workingGraph) {
        startShiftPlot = true;
    }
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
void MainWindow::on_actionShowWidgets_triggered() {
//    if (widgets == nullptr) {
//        widgets = new DialogWidgets(serialPort, this);
//        connect(widgets, SIGNAL( messageSent(QString)), this, SLOT(messageSent(QString)));
//        widgets->setWindowTitle("Widgets");
//        widgets->show();
//    }
//    else {
//        if (widgets->isHidden()) {
//            widgets->show();
//        } else {
//            widgets->hide();
//        }
//    }
    if (widgetsW == nullptr) {
        widgetsW = new widgetsWindow(serialPort, this);
        connect(widgetsW, SIGNAL( messageSent(QString)), this, SLOT(messageSent(QString)));
        widgetsW->setWindowTitle("Widgets");
        widgetsW->show();
    }
    else {
        if (widgetsW->isHidden()) {
            widgetsW->show();
        } else {
            widgetsW->hide();
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
            } else {
                tracer->setPen(QPen(Qt::white));
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
        logFile = new QFile(fileName);
        if (!logFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     logFile->errorString());
            logFile = nullptr;
            return;
        }
        streamLog.setDevice(logFile);
        // Set Headers
        streamLog << "NAME" << ";" << "POINT" << ";" << "VALUE" << ";" << "TIME" << "\n";
        ui->logPlotButton->setText("Stop Logging");
    } else {
        ui->logPlotButton->setText("Log Displayed Plots");
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
}

/******************************************************************************************************************/
void MainWindow::doContextMenuHeader(yaspGraph* yGraph) {
    contextMenu->setAttribute(Qt::WA_TranslucentBackground);
    contextMenu->setStyleSheet("QMenu {border-radius:16px;}");
    QWidget* menuTitleWidget = new QWidget();
    QLabel* menuTitleLabel = new QLabel(yGraph->plot()->name(), contextMenu);
    menuTitleLabel->setAlignment(Qt::AlignCenter);
    QLabel* menuIcon = new QLabel();
    menuIcon->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png").scaledToWidth(300));
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

/******************************************************************************************************************/
void MainWindow::infoModeLabelSelectionChanged(bool b) {
    // Unselect All Graphs and reset Command
    if (b) {
        Q_ASSERT(workingGraph);
        unselectGraphs();
    }
}

/******************************************************************************************************************/
void MainWindow::plotLabelSelectionChanged(bool b) {
//    qDebug() << "plotLabelSelectionChanged : " << b;
//    qDebug() << "plotLabelSelectionChanged selectedItems : " << ui->plot->selectedItems().size();
    if (b) {
        contextMenu->clear();
        if (ui->plot->selectedItems().size()) {
            Q_ASSERT(ui->plot->selectedItems().size() == 1);
            QCPAbstractItem* item = ui->plot->selectedItems().at(0);
            QVariant plotId = item->property("id");
            yaspGraph* yGraph = graphs[plotId.toInt()];
            Q_ASSERT(yGraph);
            if (workingGraph && (workingGraph != yGraph)) {
                workingGraph->setSelected(false);
                resetTracer();
            }
            workingGraph = yGraph;
            contextMenu->setProperty("id", plotId);
            selectedPlotId = plotId.toInt();
        }
        Q_ASSERT(workingGraph);
        workingGraph->setSelected(true);
        doContextMenuHeader(workingGraph);
        QAction* action;
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
        if (measureMode == measureType::None) {
            if (workingGraph->plot()->visible()) {
                action = contextMenu->addAction("Color", this, SLOT(doMenuPlotColorAction()));
                action->setIcon(QIcon(":/Icons/Icons/icons8-paint-palette-48.png"));
#ifdef YASP_PLOTS_WIDTH
                QWidgetAction* widgetAction = new QWidgetAction(contextMenu);
                QWidget *popup = new QWidget(this);
                QSlider* slider = new QSlider(Qt::Horizontal, popup);
                slider->setRange(1, 5);
                slider->setSingleStep(1);
                slider->setPageStep(1);
                slider->setValue(workingGraph->plot()->pen().width());
                slider->setStyleSheet("border:2px solid white");
                connect(slider, SIGNAL(valueChanged(int)), this, SLOT(doMenuPlotWidthAction(int)));
                QLabel* label = new QLabel(popup);
                label->setAlignment(Qt::AlignCenter);
                label->setText("Pen Width");
                QLabel* labelIcon = new QLabel(popup);
                QPixmap labelPixMap = QPixmap(":/Icons/Icons/icons8-line-width-40.png").scaled(24,24);
                labelIcon->setPixmap(labelPixMap);
                label->setScaledContents(true);
                labelIcon->setScaledContents(true);
//                label->setMinimumWidth(label->sizeHint().width());
                QHBoxLayout* hBoxLayout = new QHBoxLayout(popup);
                hBoxLayout->setContentsMargins(4,0,10,0);
                //hBoxLayout->setMargin(10);
                hBoxLayout->addWidget(labelIcon);
                hBoxLayout->addWidget(label);
                hBoxLayout->addWidget(slider);
                widgetAction->setDefaultWidget(popup);
                contextMenu->addAction(widgetAction);
#endif
            }
            plotShowHideAction = contextMenu->addAction("Hide", this, SLOT(doMenuPlotShowHideAction()));
            if (workingGraph->plot()->visible()) {
                plotShowHideAction->setText("Hide");
//                qDebug() << " ---- SHOW MODE ---- " << __FUNCTION__ << " / " <<  __LINE__;
                infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");
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
            if (workingGraph->plot()->visible()) {
                action = contextMenu->addAction("Reset", this, SLOT(doMenuPlotResetAction()));
                action->setIcon(QIcon(":/Icons/Icons/icons8-available-updates-40.png"));
                action = contextMenu->addAction("Save", this, SLOT(saveSelectedGraph()));
                action->setIcon(QIcon(":/Icons/Icons/icons8-save-48.png"));
                if (plotting == false) {
                    plotMeasureBoxAction = contextMenu->addAction("Start Measure Box", this, SLOT(doMenuPlotMeasureBoxAction()));
                    plotMeasureBoxAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
                    plotMeasureAction = contextMenu->addAction("Start Measure Arrow", this, SLOT(doMenuPlotMeasureAction()));
                    plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-design-40.png"));
                    plotMeasureAction = contextMenu->addAction("Start Measure Freq", this, SLOT(doMenuPlotMeasureFreqAction()));
                    plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-sonometer-48.png"));
                } else if (ui->checkBoxDynamicMeasures->isChecked()) {
                    plotMeasureBoxAction = contextMenu->addAction("Start Measure Box", this, SLOT(doMenuPlotMeasureBoxAction()));
                    plotMeasureBoxAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
                    plotMeasureAction = contextMenu->addAction("Start Measure Arrow", this, SLOT(doMenuPlotMeasureAction()));
                    plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-design-40.png"));
//                    plotMeasureAction = contextMenu->addAction("Start Measure Freq", this, SLOT(doMenuPlotMeasureFreqAction()));
//                    plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-sonometer-48.png"));
                }
            }
        } else {
            plotMeasureBoxAction = contextMenu->addAction("Stop Measure", this, SLOT(doMenuPlotMeasureBoxAction()));
            plotMeasureBoxAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
        }
        ui->plot->setContextMenuPolicy(Qt::CustomContextMenu);
    }
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::xAxisRangeChanged(const QCPRange& range) {
    double val = range.upper - range.lower;
    if (!qFuzzyCompare(val, ui->spinDisplayTime->value())) {
//         qDebug() << "xAxisRangeChanged " << val << " / " << ui->spinDisplayTime->value();
        ui->spinDisplayTime->setValue(val);
        if (workingGraph) {
            if (measureMode == measureType::Box) {
                updateTracerBox();
            } else if (measureMode == measureType::Arrow) {
                updateTracerMeasure();
            } else if (measureMode == measureType::Freq) {
                updateTracerFrequency();
            }
        }
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::yAxisRangeChanged(const QCPRange& range) {
    ui->spinDisplayRange->setValue(range.size());
    if (workingGraph) {
        if (measureMode == measureType::Box) {
            updateTracerBox();
        } else if (measureMode == measureType::Arrow) {
            updateTracerMeasure();
        } else if (measureMode == measureType::Freq) {
            updateTracerFrequency();
        }
    }
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::checkBoxDynamicMeasuresChanged(int state) {
    Q_UNUSED(state);
    if (workingGraph) {
        if (measureMode != measureType::None) {
            resetTracer();
        }
        plotLabelSelectionChanged(true);
    }
    measureMode = measureType::None;
}

/******************************************************************************************************************/
void MainWindow::on_spinDisplayRange_valueChanged(double arg1) {
//    qDebug() << "-----------------------------------------";
    QCPRange range = ui->plot->yAxis->range();
    double ratio = arg1 / range.size();
    ui->plot->yAxis->setRange(range.lower * ratio, range.upper * ratio);
}


/******************************************************************************************************************/
void MainWindow::on_saveTermButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"),
                               "",
                               tr("Logs (*.log)"));
    QTextDocumentWriter writer(fileName);
    writer.setFormat("plaintext");
    bool success = writer.write(ui->receiveTerminal->document());
    if (success) {
        ui->receiveTerminal->document()->setModified(false);
        ui->statusBar->setStyleSheet("background-color:SpringGreen; font-weight:bold;");
        statusBar()->showMessage(tr("Log File: \"%1\" Saved").arg(QDir::toNativeSeparators(fileName)));
    } else {
        ui->statusBar->setStyleSheet("background-color:Tomato; font-weight:bold;");
        statusBar()->showMessage(tr("Could not write to file \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    }
}

/******************************************************************************************************************/
void MainWindow::on_restartDeviceButton_clicked() {
    cleanGraphs();
    initTracer();
    on_resetPlotButton_clicked();
    serialPort->setDataTerminalReady(false);
    serialPort->setDataTerminalReady(true);
}

/******************************************************************************************************************/
void MainWindow::on_tabWidget_currentChanged(int index) {
    if (index == 0) {
        ui->statusBar->showMessage("About");
    }
}

/******************************************************************************************************************/
void MainWindow::on_aboutNevVersionButton_clicked() {
    QDesktopServices::openUrl(QUrl (downloadUrl));
}

/******************************************************************************************************************/
void MainWindow::on_actionOnline_Documentation_triggered() {
    QDesktopServices::openUrl(QUrl(DOC_URL, QUrl::TolerantMode));
}

/******************************************************************************************************************/
void MainWindow::on_autoScrollCheckBox_stateChanged(int arg1) {
    if (arg1) {
        updateTimer.start();
    } else {
        updateTimer.stop();
    }

}
