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

static const QString DEFS_URL = "https://www.devlabnet.eu/softdev/yasp/updates.json";
static const QString YASP_VERSION = "1.0";

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

//    ui->terminalWidget->setVisible(false);
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
//    plotTimeInSeconds = 30;
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
    ui->tabWidget->setCurrentIndex(0);
    ui->spinDisplayTime->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->spinDisplayTime->setMinimum(PLOT_TIME_MIN_DEF);
    ui->spinDisplayTime->setMaximum(PLOT_TIME_MAX_DEF);
//    ui->spinDisplayTime->setSingleStep(PLOT_TIME_STEP_DEF);
    ui->spinDisplayTime->setValue(PLOT_TIME_DEF);
    ui->spinDisplayTime->setDecimals(0);
    ui->spinDisplayTime->setSuffix(" Millis");
    ui->spinDisplayTime->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    ui->spinDisplayRange->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->spinDisplayRange->setMinimum(1);
    ui->spinDisplayRange->setMaximum(YAXIS_MAX_RANGE);
//    ui->spinDisplayTime->setSingleStep(PLOT_TIME_STEP_DEF);
    ui->spinDisplayRange->setValue(DEF_YAXIS_RANGE);
    ui->spinDisplayRange->setDecimals(0);
//    ui->spinDisplayRange->setSuffix(" Millis");
    ui->spinDisplayRange->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);


    ui->autoScrollLabel->setStyleSheet("QLabel { color : DodgerBlue; }");
    ui->autoScrollLabel->setText("Auto Scroll OFF, To allow move cursor to the end or SELECT Button ---> ");
    ui->tabWidget->removeTab(1);
    contextMenu = new QMenu(this);
//    connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuTriggered(QAction*)));
//    connect(contextMenu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);    
    mouseWheelTimer.stop();

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
        } else if (b > a) {
            return false;
        }
    }
    return versionsY.count() < versionsX.count();
}

/******************************************************************************************************************/
void MainWindow::loadHelpFile() {
//    ui->helpTextWidget->setUrl(QUrl(QStringLiteral("https://www.qt.io")));
 }

/******************************************************************************************************************/
void MainWindow::setUpdateAvailable(bool available, QString latestVersion, QString downloadUrl, QString changelog) {
    QMessageBox box;
    box.setIcon (QMessageBox::Information);
    box.setTextFormat(Qt::RichText);
    if (available) {
        QString text = "<h3>"
                        + tr ("Version %1 of %2 has been released!")
                        .arg (latestVersion).arg(qApp->applicationName())
                        + "</h3>";
        text += tr ("Would you like to download the update now?");
        text = text.replace(" ","&nbsp;");
        text += "<hr/><b>New in version " + latestVersion + "</b>";
        text += changelog;
        text += "<hr/><br>";
//        qDebug().noquote() << text;
        box.setText(text);
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::Yes);
        if (box.exec() == QMessageBox::Yes) {
           QDesktopServices::openUrl (QUrl (downloadUrl));
        }
    } else  {
        box.setStandardButtons (QMessageBox::Close);
        QString text = "<h3>"
                     + tr ("Congratulations!<br>You are running the "
                           "latest version of %1").arg(qApp->applicationName())
                     + "</h3>";
        text += tr("No updates are available for the moment");
        text = text.replace(" ","&nbsp;");
        box.setText(text);
        box.exec();
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
            return;
        }
        QString platformKey = "windows";
        /* Get the platform information */
        QJsonObject updates = document.object().value ("updates").toObject();
        QJsonObject platform = updates.value (platformKey).toObject();
        /* Get update information */
        QString changelog = platform.value ("changelog").toString();
        QString downloadUrl = platform.value ("download-url").toString();
        QString latestVersion = platform.value ("latest-version").toString();
        qDebug() << "changelog: " << changelog;
        qDebug() << "downloadUrl: " << downloadUrl;
        qDebug() << "latestVersion: " << latestVersion;
        setUpdateAvailable(compareVersions(latestVersion, YASP_VERSION), latestVersion, downloadUrl, changelog);
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
//    cleanDataTtime += PLOT_TIME_MAX_DEF;
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
        QCPItemLine* axisLine = new QCPItemLine(ui->plot);
        axisLine->setSelectable(false);
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

    loadHelpFile();

}

/******************************************************************************************************************/
/* Connect Button clicked slot; handles connect and disconnect */
/******************************************************************************************************************/
void MainWindow::on_connectButton_clicked() {
    if (connected) {
        closePort();
    } else {             // If application is not connected, connect
        openPort();      // Open serial port and connect its signals
    }
}

/******************************************************************************************************************/
void MainWindow::initTracer() {
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
//    updateTimer.start();
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
    traceLineBottom = new QCPItemLine(ui->plot);
    traceLineBottom->setSelectable(false);
    pen.setColor(Qt::green);
    traceLineBottom->setPen(pen);
    traceLineTop = new QCPItemLine(ui->plot);
    traceLineTop->setSelectable(false);
    pen.setColor(Qt::red);
    traceLineTop->setPen(pen);
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
    cleanTracer();
}

/******************************************************************************************************************/
/* Slot for port opened successfully */
/******************************************************************************************************************/
void MainWindow::portOpenedSuccess() {
    connect(serialPort, SIGNAL(dataTerminalReadyChanged(bool)), this, SLOT(dataTerminalReadyChanged(bool)));
    serialPort->setDataTerminalReady(false);
//    ui->terminalWidget->setVisible(true);
    ui->menuWidgets->menuAction()->setVisible(true);
    if ((widgets != nullptr)) {
        widgets->setSerialPort(serialPort);
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
    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    connect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    QWidget* tabW = ui->tabWidget->findChild<QWidget *>("tabPlots");
    ui->tabWidget->insertTab(1, tabW, "Plots");
    ui->tabWidget->setCurrentIndex(1);
    initTracer();
    updateTimer.start();
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    lastDataTtime = 0;
//    cleanDataTtime = PLOT_TIME_MAX_DEF;
    connect(&mouseWheelTimer, SIGNAL(timeout()), this, SLOT(mouseWheelTimerShoot()));
}

/******************************************************************************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape ) {
        if (workingGraph) {
            if ((measureMode == measureType::Measure) || (measureMode == measureType::Box)) {
                measureMode = measureType::None;
                cleanTracer();
                plotLabelSelectionChanged(true);
                return;
            } else {
                unselectGraphs();
            }
        }
    }
    if( event->modifiers() == Qt::ShiftModifier ) {
        if (measureMode == measureType::Measure) {
//            qDebug() << "keyPressEvent ShiftModifier";
            tracerArrowFromRef->setVisible(false);
            tracerArrowFromRefTxt->setVisible(false);
            tracerArrowAmplitudeTopTxt->setVisible(false);
            tracerArrowAmplitudeBottomTxt->setVisible(false);
        }
    }
    if( event->modifiers() == Qt::ControlModifier ) {
        if (measureMode == measureType::Measure) {
//            qDebug() << "keyPressEvent ControlModifier";
            tracerArrowFromRef->setVisible(true);
            tracerArrowFromRefTxt->setVisible(true);
            tracerArrowAmplitudeTopTxt->setVisible(true);
            tracerArrowAmplitudeBottomTxt->setVisible(true);
            tracerArrowFromRef->start->setCoords(tracer->position->coords());
            tracerArrowFromRef->end->setCoords(tracer->position->coords());
        }
    }
}

/******************************************************************************************************************/
void MainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   if (infoModeLabel && infoModeLabel->visible()) {
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
    connected = false;
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    disconnect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    disconnect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    serialPort->close();                                                              // Close serial port
    delete serialPort;                                                                // Delete the pointer
    serialPort = nullptr;                                                                // Assign NULL to dangling pointer
    ui->connectButton->setText("Connect");                                            // Change Connect button text, to indicate disconnected
    ui->statusBar->setStyleSheet("background-color: SkyBlue; font-weight:bold;");
    ui->statusBar->showMessage("Disconnected!");                                      // Show message in status bar
//    ui->terminalWidget->setVisible(false);
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
            ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
            ui->plot->replot();
        } else {
            ui->plot->xAxis->setRange(ui->plot->xAxis->range());
//            updateTracerMeasure();
//            updateTracerBox();
        }
    }
}

/******************************************************************************************************************/
void MainWindow::mouseWheelTimerShoot() {
//    mouseWheelTimer.setSingleShot(true);
    mouseWheelTimer.stop();
    if (workingGraph) {
//        qDebug() << "<<<<<<<<<<<<< mouseWheelTimerShoot >>>>>>>>>>>>>>";
        if (measureMode == measureType::Box) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        } else if (measureMode == measureType::Measure) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE MODE");
        } else {
//            qDebug() << " ---- SHOW MODE ---- " << __FUNCTION__ << " / " <<  __LINE__;
            infoModeLabel->setText(workingGraph->plot()->name() + " -> SHOW MODE");
        }
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    }
}

/******************************************************************************************************************/
void MainWindow::unselectGraphs() {
//    qDebug() << "<<<<<<<<<<<<< unselectGraphs >>>>>>>>>>>>>>";
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
//        updateTimer.stop();
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
    } else {
        // Start plotting
        // Start updating plot timer
//        updateTimer.start();
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
            double currentTime = newData[1].toDouble();
            if (currentTime < lastDataTtime) {
                // Will normally never (or rarely) append
                // Means that current millis() returned is lower than the previous one !!
                // --> millis() overflow !!
                // So just clean everything in graph
                qDebug() << currentTime <<  " ============================ CLEAN OVERFLOW ============================ " << lastDataTtime;
//                qDebug() << "============================ CLEAN OVERFLOW ============================";
//                qDebug() << "============================ CLEAN OVERFLOW ============================";
                cleanDataGraphs();
            }
            lastDataTtime = currentTime;
//            if (lastDataTtime > cleanDataTtime) {
                // Clean graphs data
                cleanDataGraphsBefore(lastDataTtime);
//            }
            double val = newData[2].toDouble();
            val *= yGraph->mult();
            val += yGraph->offset();
            // Add data to graphs according plot Id
            QCPGraph* plot = yGraph->plot();
            QString plotInfoStr = " val: ";
            plotInfoStr += QString::number(val, 'f', 3);
            plotInfoStr +=  + " offset: ";
            plotInfoStr += QString::number(yGraph->offset(), 'f', 3);
            plotInfoStr +=  + " mult: ";
            plotInfoStr += QString::number(yGraph->mult(), 'f', 3);
            updateLabel(plotId, plotInfoStr);
            plot->addData(lastDataTtime, val );
            ui->dataInfoLabel->setNum(dataPointNumber);
        } else {
            qDebug() << "------------> BAD DATA : " << plotId << " / " << dataListSize << " --> " << newData;
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
//                    qDebug() << "++++ " << data;
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
    plotTimeInSeconds = PLOT_TIME_DEF;
    ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE/2, DEF_YAXIS_RANGE/2);       // Set lower and upper plot range
    ui->spinDisplayRange->setValue(ui->plot->yAxis->range().size());
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    ui->plot->replot();
}

/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinDisplayTime_valueChanged(double arg1) {
    plotTimeInSeconds = arg1;
//    if (plotTimeInSeconds < 100) {
//        ui->spinDisplayTime->setSingleStep(1);
//    } else {
//        int log = static_cast<int>(log10(plotTimeInSeconds)) - 1;
//        ui->spinDisplayTime->setSingleStep(static_cast<int>(pow(10, log)));
//    }
    if (plotting) {
        ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
    } else {
        double oldRange = ui->plot->xAxis->range().size();
        double inc = (plotTimeInSeconds - oldRange)/2.0;
        ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower - inc, ui->plot->xAxis->range().upper + inc);
    }
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
    Q_ASSERT(workingGraph);
    workingGraph->setMult(scale);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::cleanTracer() {
//    qDebug() << "========================== cleanTracer ==========================";
    ui->plot->setCursor(Qt::ArrowCursor);
    measureMode = measureType::None;
    tracer->setVisible(false);
    togglePlotsVisibility(true);
    infoModeLabel->setVisible(false);
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
//    qDebug() << "ShowPlotsExceptWG: " << workingGraph->plot()->name() << " show " << show;
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
        cleanTracer();
        measureMode = measureType::None;
        plotLabelSelectionChanged(true);
    } else {
//        qDebug() << "Init Measure !";
        measureMode = measureType::Measure;
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
void MainWindow::doMenuPlotMeasureBoxAction() {
//    qDebug() << "doMenuPlotMeasureBoxAction: " << contextMenu->property("id") << " measureInProgress: " << measureInProgress;
    Q_ASSERT(contextMenu);
    Q_ASSERT(workingGraph);
//    if (measureInProgress) {
    if (measureMode != measureType::None) {
        cleanTracer();
//        qDebug() << " ---- resetMouseWheelState false ---- " << __FUNCTION__ << " / " <<  __LINE__;
        measureMode = measureType::None;
        plotLabelSelectionChanged(true);
    } else {
//        qDebug() << "Init Tracer !";
//        qDebug() << " ---- resetMouseWheelState true ---- " << __FUNCTION__ << " / " <<  __LINE__;
        measureMode = measureType::Box;
        ShowPlotsExceptWG(false);
        measureMult = workingGraph->mult();
//        ui->plot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag );
        infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
        plotLabelSelectionChanged(true);
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
        ui->plot->setCursor(Qt::CrossCursor);
        tracer->setVisible(true);
        tracer->setGraph(workingGraph->plot());
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
//        qDebug() << "doMenuPlotColorAction: " << color;
        QPen pen = workingGraph->plot()->pen();
        pen.setColor(color);
        workingGraph->plot()->setPen(pen);
        workingGraph->setSelected(true);
    }
    resetMouseWheelState();
    infoModeLabel->setText(workingGraph->plot()->name() + " --> MENU MODE");
}

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
    qDebug() << "doMenuPlotShowHideAction: " << workingGraph->plot()->name();
    resetMouseWheelState();
    Q_ASSERT(workingGraph);
    if (workingGraph->plot()->visible()) {
        workingGraph->hide(true);
        workingGraph->rLine()->setVisible(false);
        plotShowHideAction->setText("show");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
        infoModeLabel->setText(workingGraph->plot()->name() + " --> HIDE MODE");
        infoModeLabel->setVisible(true);
        infoModeLabel->setColor(workingGraph->plot()->pen().color());
    } else {
        workingGraph->hide(false);
        workingGraph->rLine()->setVisible(true);
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
void MainWindow::updateTracerMeasure(bool adjustHeight, double scale) {
    if (measureMode == measureType::Measure) {
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
        double offset = workingGraph->offset();
        if (adjustHeight) {
            startY -= offset;
            startY *= scale;
            startY += offset;
            tracerArrowFromRef->start->setCoords(startX, startY);
        }
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
void MainWindow::updateTracerBox(bool adjustHeight, double scale) {
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
        double ref = workingGraph->rLine()->start->coords().y();
//        double plotVCenter = ui->plot->yAxis->range().center() * 0.8;
        double space = plotWidth / 800;
        double coordX = ui->plot->xAxis->pixelToCoord(pX);
        if (adjustHeight) {
//            double botToTop = traceLineTop->start->coords().y() - traceLineBottom->start->coords().y();
            double refToTop = traceLineTop->start->coords().y() - ref;
            double botToRef = ref - traceLineBottom->start->coords().y();
//            qDebug() << "         BOT Y " << botToTop << " / refToTop " << refToTop << " / botToRef " << botToRef;
            refToTop *= scale;
            botToRef *= scale;
//            qDebug() << "ADJUSTED BOT Y " << botToTop << " / refToTop " << refToTop << " / botToRef " << botToRef;
            double adjustedEndY = ref + refToTop;
            traceLineTop->start->setCoords(0, adjustedEndY);
            traceLineTop->end->setCoords(DBL_MAX, adjustedEndY);
//            qDebug() << "BOT Y " << traceLineBottom->start->coords().y() << " / Scale " << scale << " / adjustedEndY " << adjustedEndY;
            adjustedEndY = ref - botToRef;
            traceLineBottom->start->setCoords(0, adjustedEndY);
            traceLineBottom->end->setCoords(DBL_MAX, adjustedEndY);
//            qDebug() << "TOP Y " << traceLineTop->start->coords().y() << " / Scale " << scale << " / adjustedEndY " << adjustedEndY;
        }
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
        if (traceLineBottom->start->coords().y() > endY) {
//            qDebug() << "traceLineBottom "  << endY;
            traceLineBottom->start->setCoords(0, endY);
            traceLineBottom->end->setCoords(DBL_MAX, endY);
        }
        if (traceLineTop->start->coords().y() < endY) {
//            qDebug() << "traceLineTop "  << endY;
            traceLineTop->start->setCoords(0, endY);
            traceLineTop->end->setCoords(DBL_MAX, endY);
        }
        if (qFuzzyCompare(traceLineTop->start->coords().y(), traceLineBottom->start->coords().y())) {
            if (traceLineBottom->start->coords().y() > ref) {
                traceLineBottom->start->setCoords(0, ref);
            }
            if (traceLineTop->start->coords().y() < ref) {
                traceLineTop->start->setCoords(0, ref);
            }
        }
        double plotVCenter = (traceLineTop->start->coords().y() + traceLineBottom->start->coords().y()) / 2.0;
//        qDebug() << "plotVCenter "  << plotVCenter;
        double rh = plotHeight * 0.35;
        double tracerRectTop = plotVCenter + rh;
        double tracerRectBottom = plotVCenter - rh;
        double tracerRectLeft = endX - (plotWidth/3);
        double tracerRectRight = endX + (plotWidth/3);
        tracerRect->topLeft->setCoords(tracerRectLeft, tracerRectTop);
        tracerRect->bottomRight->setCoords(tracerRectRight, tracerRectBottom);
        // Vertical measures
        double amplitude = traceLineTop->start->coords().y() - traceLineBottom->start->coords().y();
        double deltaTop = traceLineTop->start->coords().y() - endY;
        double deltaBottom = endY - traceLineBottom->start->coords().y();
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
        ui->statusBar->setStyleSheet("background-color: lightgreen; font-weight:bold;");
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
        for (QCPDataContainer<QCPGraphData>::const_iterator it = itStart; it != itEnd; ++it) {
            double deltaFromRef = abs(ref - (it->value))/mult;
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
//           if (compareDouble( it->value, endY, 0)) {
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
//    QString coordinates("Time: %1 Seconds -- Y: %2 -- Points: %3");
//    coordinates = coordinates.arg(xx/1000.0,0,'f',3).arg(yy,0,'f',3).arg(dataPointNumber);
    QString coordinates(" -- Time: %1 Seconds -- Amplitude: %2 -- ");
    coordinates = coordinates.arg(xx/1000.0,0,'f',3).arg(yy,0,'f',3);
    ui->statusBar->setStyleSheet("background-color: SkyBlue; font-weight:bold;");
    if (measureMode == measureType::Box) {
        if (event->buttons() != Qt::LeftButton) {
            updateTracerBox();
        } else {
            // shift all plots
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
            ui->statusBar->showMessage(coordinates);
        }
    } else if (measureMode == measureType::Measure) {
        if (event->buttons() != Qt::LeftButton) {
            updateTracerMeasure();
        } else {
            // shift all plots
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
            ui->statusBar->showMessage(coordinates);
        }
    } else {
        if (workingGraph) {
            ui->plot->setInteractions(QCP::iRangeZoom | QCP::iSelectItems );
            // Just shift the selected plot
            if (event->buttons() == Qt::LeftButton) {
                infoModeLabel->setText(workingGraph->plot()->name() + " -> SHIFT MODE");
                infoModeLabel->setColor(workingGraph->plot()->pen().color());
                infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
                shiftPlot(event->y());
                QString msg = "Moving PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->offset(), 'f', 3);
                ui->statusBar->showMessage(msg);
            }
        } else {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
            ui->statusBar->showMessage(coordinates);
        }
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event) {
    Q_UNUSED(event)
//    boolTest = true;

//    qDebug() << "---------- release ---------- measureInProgress: " << measureInProgress;
    if (workingGraph) {
        startShiftPlot = false;
        if (measureMode == measureType::Box) {
            infoModeLabel->setText(workingGraph->plot()->name() + " -> MEASURE BOX MODE");
        } else {
//            qDebug() << " ---- SHOW MODE ---- " << __FUNCTION__ << " / " <<  __LINE__;
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
//    qDebug() << "resetMouseWheelState: " << measureInProgress;
//    qDebug() << " ---- resetMouseWheelState false ---- " << __FUNCTION__ << " / " <<  __LINE__;
    mouseWheelTimer.stop();
    startShiftPlot = false;
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - 32, 16);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::onMouseWheelInPlot(QWheelEvent *event) {
//    mousePos = event->pos();
    if (event->buttons() == Qt::RightButton) {
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
        QPoint numDegrees = event->angleDelta();
        if (numDegrees.y() == 0) return;
        double inc = plotTimeInSeconds / 100.0;

        if (numDegrees.y() > 0) {
            plotTimeInSeconds += inc;
        } else {
            plotTimeInSeconds -= inc;
        }
        if (plotting) {
            ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);
        } else {
            if (numDegrees.y() > 0) {
                ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower - inc, ui->plot->xAxis->range().upper + inc);
            } else {
                ui->plot->xAxis->setRange(ui->plot->xAxis->range().lower + inc, ui->plot->xAxis->range().upper - inc);
            }
            ui->plot->replot();
        }
    } else {
        // zoom display
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        if (workingGraph) {
//            if (measureMode == measureType::None) {
                ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
                QPoint numDegrees = event->angleDelta();
                int nY = numDegrees.y();
                if (nY != 0) {
                    mouseWheelTimer.start(500);
                    infoModeLabel->setText(workingGraph->plot()->name() + " -> SCALE MODE");
                    double scale = 1 + (static_cast<double>(nY) / 1000.0);
                    scalePlot(scale);
                    QString msg = "Scaling PLOT: " + workingGraph->plot()->name() + " -> " + QString::number(workingGraph->mult(), 'f', 3);
                    ui->statusBar->showMessage(msg);
                    if (measureMode == measureType::Measure) {
                        updateTracerMeasure(true, scale);
                    }
                    if (measureMode == measureType::Box) {
                        if (nY != 0) {
                            double scale = 1 + (static_cast<double>(nY) / 1000.0);
                            updateTracerBox(true, scale);
                        }
                    }
                }
//            }
        } else {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
        }
    }
}

/******************************************************************************************************************/
void MainWindow::onMousePressInPlot(QMouseEvent *event) {
    Q_UNUSED(event);
//    if( event->modifiers() & Qt::ShiftModifier ) {
//        if (measureMode == measureType::Measure) {
//            qDebug() << "onMousePressInPlot ShiftModifier";
//            tracerArrowFromRef->setVisible(false);
//        }
//    }

//    if( event->modifiers() & Qt::ControlModifier ) {
//        if (measureMode == measureType::Measure) {
//            qDebug() << "onMousePressInPlot ControlModifier";
//            tracerArrowFromRef->setVisible(true);
//            tracerArrowFromRef->start->setCoords(tracer->position->coords());
//            tracerArrowFromRef->end->setCoords(tracer->position->coords());
//        }
//    }

    //    qDebug() << "onMousePressInPlot " << event->button()
//             << " measureInProgress: " << measureInProgress
//             << " menu: " << contextMenu->isVisible();
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
//        qDebug() << "Log Plot : " << fileName;
        logFile = new QFile(fileName);
        if (!logFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     logFile->errorString());
            logFile = nullptr;
            return;
        }
//        qDebug() << "Log Plot Opened : " << logFile->fileName();
        streamLog.setDevice(logFile);
        // Set Headers
        streamLog << "NAME" << ";" << "POINT" << ";" << "VALUE" << ";" << "TIME" << "\n";
        ui->logPlotButton->setText("Stop Logging");
    } else {
//        qDebug() << "Log Plot Closed : " << logFile->fileName();
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
    QLabel* menuTitleLabel = new QLabel("plot " + yGraph->plot()->name(), contextMenu);
    menuTitleLabel->setAlignment(Qt::AlignCenter);
    QLabel* menuIcon = new QLabel();
    menuIcon->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png").scaledToWidth(300));
//    menuIcon->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png"));
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
//    qDebug() << "infoModeLabelSelectionChanged : " << ui->plot->selectedItems().size();
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
                cleanTracer();
                resetMouseWheelState();
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
    //            action = contextMenu->addAction("Scale", this, SLOT(doMenuPlotScaleAction()));
    //            action->setIcon(QIcon(":/Icons/Icons/icons8-height-48.png"));
    //            action = contextMenu->addAction("Shift", this, SLOT(doMenuPlotShiftAction()));
    //            action->setIcon(QIcon(":/Icons/Icons/icons8-shift-48.png"));
                action = contextMenu->addAction("Save", this, SLOT(saveSelectedGraph()));
                action->setIcon(QIcon(":/Icons/Icons/icons8-save-48.png"));
                if ((plotting == false) || (ui->checkBoxDynamicMeasures->isChecked())) {
                    plotMeasureBoxAction = contextMenu->addAction("Start Measure Box", this, SLOT(doMenuPlotMeasureBoxAction()));
                    plotMeasureBoxAction->setIcon(QIcon(":/Icons/Icons/icons8-caliper-48.png"));
                    plotMeasureAction = contextMenu->addAction("Start Measure", this, SLOT(doMenuPlotMeasureAction()));
                    plotMeasureAction->setIcon(QIcon(":/Icons/Icons/icons8-design-40.png"));
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
    }
    updateTracerBox();
    updateTracerMeasure();
}

/******************************************************************************************************************/
void MainWindow::yAxisRangeChanged(const QCPRange& range) {
    ui->spinDisplayRange->setValue(range.size());
    updateTracerBox();
    updateTracerMeasure();
}

/******************************************************************************************************************/
void MainWindow::checkBoxDynamicMeasuresChanged(int state) {
    Q_UNUSED(state);
//    qDebug() << "checkBoxDynamicMeasuresChanged " << state;
//    qDebug() << " selectedItems: " << ui->plot->selectedItems();
//    qDebug() << " selectedGraphs: " << ui->plot->selectedGraphs();
//    qDebug() << " selectedPlottables: " << ui->plot->selectedPlottables();
//    qDebug() << " workingGraph: " << workingGraph;
    if (workingGraph) {
        if ((measureMode == measureType::Measure) || (measureMode == measureType::Box)) {
            cleanTracer();
        }
        plotLabelSelectionChanged(true);
    }
    measureMode = measureType::None;
    if (state) {
    } else {
    }
}

/******************************************************************************************************************/
void MainWindow::on_spinDisplayRange_valueChanged(double arg1) {
//    qDebug() << "-----------------------------------------";
    QCPRange range = ui->plot->yAxis->range();
//    double lower = ui->plot->yAxis->range().lower;
//    double upper = ui->plot->yAxis->range().upper;
//    qDebug() << "arg1 " << arg1 << " / range " << range.size() << " lower " << lower << " upper " << upper;
    double ratio = arg1 / range.size();
    ui->plot->yAxis->setRange(range.lower * ratio, range.upper * ratio);
//    range = ui->plot->yAxis->range();
//    lower = ui->plot->yAxis->range().lower;
//    upper = ui->plot->yAxis->range().upper;
//    qDebug() << "ratio " << ratio << " / range " << range.size() << " lower " << lower << " upper " << upper;

}

/******************************************************************************************************************/
void MainWindow::on_saveTermButton_clicked() {
    qDebug() << "----------------SAVE TERM TBD ------------------";
}

/******************************************************************************************************************/
void MainWindow::on_restartDeviceButton_clicked() {
    qDebug() << "----------------RESTART TBD ------------------";
}

/******************************************************************************************************************/
void MainWindow::on_tabWidget_currentChanged(int index) {
    qDebug() << "on_tabWidget_currentChanged " << index;
//    if (index == 0) {
//        ui->helpTextWidget->setVisible(true);
//        ui->terminalWidget->setVisible(false);
//        ui->plot->setVisible(false);
////        QWidget* ww = new QWidget();
////        QVBoxLayout* vbl = new QVBoxLayout;
////        QLabel* label = new QLabel();
////        QPixmap imagePixmap;
////        imagePixmap.load(":/Icons/Icons/Oscilloscope-128.png");
////        label->setPixmap(imagePixmap);
////        label->setFixedSize(100,100);
////        //label->setScaledContents(true);
////        vbl->addWidget(label);
////        ww->setLayout(vbl);
////        ui->terminalWidget->setParent(ww);
//////        ui->terminalWidget->setFixedSize(0,0);
//////        ui->tabPlots->setFixedSize(0,0);
//    } else {
//        ui->helpTextWidget->setVisible(false);
//        ui->terminalWidget->setVisible(true);
//        ui->plot->setVisible(true);
//    }
}
