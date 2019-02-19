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
    connect(ui->plot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(onMouseDoubleClickInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMoveInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseReleaseInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onMouseWheelInPlot(QWheelEvent*)));
    connect(ui->plot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedByUserInPlot()));
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
yaspGraph* MainWindow::addGraph(int id) {
        ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
//        ui->plot->xAxis->setRange(0, plotTimeInSeconds);                                      // Set x axis range for specified number of points
        QString plotStr = "Plot " + QString::number(id);
        QCPGraph* graph = ui->plot->addGraph();
        plotDashPattern = QVector<qreal>() << 16 << 4 << 8 << 4;
        graph->setName(plotStr);
        QCPItemText* textLabel = new QCPItemText(ui->plot);
        textLabel->setProperty("id", id);
        textLabel->setColor(colours[id]);
        textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
        textLabel->position->setType(QCPItemPosition::ptAbsolute );
        textLabel->setSelectable(true);
        textLabel->setPadding(QMargins(2,2,2,2));
        connect(textLabel, SIGNAL(selectionChanged (bool)), this, SLOT(plotLabelSelected(bool)));
        QCPItemLine* axisLine = new QCPItemLine(ui->plot);
        QPen pen = QPen(colours[id], 0.5);
        rLineDashPattern = QVector<qreal>() << 64 << 4 ;
        pen.setDashPattern(rLineDashPattern);
        axisLine->setPen(pen);
        axisLine->start->setCoords(0,0);
        axisLine->end->setCoords(plotTimeInSeconds, 0);
        yaspGraph* g = new yaspGraph(id, graph, textLabel, axisLine);
        graphs.insert(id, g);
        return g;
}

/******************************************************************************************************************/
void MainWindow::updateLabel(int id, QString info) {
    yaspGraph* yGraph = graphs[id];
    Q_ASSERT(yGraph);
    info = yGraph->plot()->name() + " -> " + info;
    QColor color = yGraph->plot()->pen().color();
    QFont font;
    font.setPointSize(7);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Medium);
    font.setStyle(QFont::StyleItalic);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(info);
    qreal pixelsHigh = fm.height();
    QCPItemText* textLabel =  yGraph->info();
    Q_ASSERT(textLabel != nullptr);
    QPoint labelPos;
    labelPos.setX(pixelsWide + textLabel->padding().left()+ textLabel->padding().right() + ui->plot->yAxis->axisRect()->left() + 70 );
    labelPos.setY( (id+1) * (5 + (pixelsHigh + textLabel->padding().top() + textLabel->padding().bottom())));
    textLabel->setColor(color);
    textLabel->setSelectedColor(color);
    textLabel->setSelectedPen(yGraph->plot()->pen());

    textLabel->position->setCoords(labelPos.x(), labelPos.y());
    textLabel->setText(info);
    QPen pen(color, 0.5);
    pen.setDashPattern(rLineDashPattern);
    yGraph->rLine()->setPen(pen);
    yGraph->info()->pen().setColor(color);
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
    infoModeLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    infoModeLabel->position->setType(QCPItemPosition::ptAbsolute );
    infoModeLabel->setPadding(QMargins(2,2,2,2));
    QFont font;
    font.setPointSize(12);
    font.setStyleHint(QFont::Monospace);
    font.setWeight(QFont::Bold);
    font.setStyle(QFont::StyleItalic);
    infoModeLabel->setFont(font);
    QFontMetricsF fm(font);
    qreal pixelsWide = fm.width(infoModeLabel->text());
    qDebug() << "pixelsWide " << pixelsWide;
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    plotLabelSelected(false);
//    qDebug() << "ui->plot->geometry: " << ui->plot->geometry();
    updateTimer.start();
    lastDataTtime = 0;
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);

//    ui->plot->xAxis->setRange(lastDataTtime - 20, lastDataTtime);

//    ticksXTimer.start();
//    ticksXTime.restart();
}

/******************************************************************************************************************/
void MainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   if (infoModeLabel) {
       QFontMetricsF fm(infoModeLabel->font());
       qreal pixelsWide = fm.width(infoModeLabel->text());
       infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
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
//        ui->plot->xAxis->setRange(dataPointNumber - plotTimeInSeconds, dataPointNumber);
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
/* Stop Plot Button */
/******************************************************************************************************************/
void MainWindow::on_stopPlotButton_clicked() {
    if (plotting) {
        // Stop plotting
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        // Stop updating plot timer
        updateTimer.stop();
//        ticksXTimer.stop();
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
    } else {
        // Start plotting
        // Start updating plot timer
        updateTimer.start();
//        ticksXTimer.start();
//        ticksXTime.restart();
        plotting = true;
        ui->stopPlotButton->setText("Stop Plot");
    }
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
        updateLabel(plotId);
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
        if (dataListSize > 1) {
            lastDataTtime = newData[1].toDouble()/1000000.0;
            double val = newData[2].toDouble();
            val *= yGraph->mult();
            val += yGraph->offset();
            // Add data to graphs according plot Id
            QCPGraph* plot = yGraph->plot();
            QString info = " val: ";
            info += QString::number(val);
            info +=  + " offset: ";
            info += QString::number(yGraph->offset());
            info +=  + " mult: ";
            info += QString::number(yGraph->mult());
            updateLabel(plotId, info);
            plot->addData(lastDataTtime, val );
//            qDebug() << "ADD DATA : " << plotId << " --> " << time << " / " << val;
//            pointTime.insert(dataPointNumber, ticksXTime.elapsed());
            yGraph->updateRefLine(dataPointNumber);
            ui->statusBar->showMessage("Points: " + QString::number(dataPointNumber));

            if (logFile != nullptr) {
                //                if (plot->isDisplayed()) {
                //                    streamLog << plot->getName() << ";" << dataPointNumber << ";" << val << ";" << time << "\n";
                //                }
            }
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
    ui->plot->deselectAll();
    plotTimeInSeconds = PLOT_TIME_DEF;
    ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);

    ui->spinDisplayTime->setValue(plotTimeInSeconds);
//    ticksXTimer.setInterval(plotTimeInSeconds / 10);
    selectionChangedByUserInPlot();
}

/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinDisplayTime_valueChanged(double arg1) {
    plotTimeInSeconds = arg1;
//    ticksXTimer.setInterval(plotTimeInSeconds / 10);
    ui->plot->xAxis->setRange(lastDataTtime - plotTimeInSeconds, lastDataTtime);

//    ui->plot->xAxis->setRange(dataPointNumber - plotTimeInSeconds, dataPointNumber);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::doMeasure() {
    Q_ASSERT(ui->plot->selectedGraphs().size());
    cleanTracer();
    measureInProgress = true;
    ui->plot->setCursor(Qt::CrossCursor);
    ui->plot->setInteraction(QCP::iSelectAxes, false);
    ui->plot->setInteraction(QCP::iSelectPlottables, false);
    tracer = new QCPItemTracer(ui->plot);
    tracer->setSize(4);
    if (bgColor.lightness() > 128) {
        tracer->setPen(QPen(Qt::black));
    } else {
        tracer->setPen(QPen(Qt::white));
    }
        QCPGraph* gr = ui->plot->selectedGraphs().at(0);
        tracer->setGraph(gr);
        ui->plot->deselectAll();
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
        workingGraph->setOffset(dataPointNumber, offset);
        ui->plot->replot();
        lastPosY = posY;
        lastY = pY;
    }
}

/******************************************************************************************************************/
void MainWindow::scalePlot(int numDeg) {
    if (numDeg == 0) return;
    if (startScalePlot) {
        scaleMult = 1.0;
        startScalePlot = false;
    }
    Q_ASSERT(workingGraph);
    double scale = 1 + ((double)numDeg / 1000.0);
    workingGraph->setMult(dataPointNumber, scale);
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::cleanTracer() {
    measureInProgress = false;
    if (tracer) {
        delete tracer;
        tracer = nullptr;
    }
    if (rubberBand) {
        delete rubberBand;
        rubberBand = nullptr;
    }
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotShiftAction() {
    Q_ASSERT(contextMenu);
    resetMouseWheelState();
//    qDebug() << "doMenuPlotShiftAction: " << contextMenu->property("id");
    int plotId = contextMenu->property("id").toInt();
    yaspGraph* yGraph = graphs[plotId];
    Q_ASSERT(yGraph);
    workingGraph = yGraph;
    mouseState = mouseShift;
    infoModeLabel->setText(yGraph->plot()->name() + " -> SHIFT MODE");
    infoModeLabel->setColor(yGraph->plot()->pen().color());
    QFontMetricsF fm(infoModeLabel->font());
    qreal pixelsWide = fm.width(infoModeLabel->text());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotColorAction() {
    Q_ASSERT(contextMenu);
    resetMouseWheelState();
    int plotId = contextMenu->property("id").toInt();
    yaspGraph* yGraph = graphs[plotId];
    Q_ASSERT(yGraph);
//    qDebug() << "doMenuPlotColorAction: " << plotId;
    infoModeLabel->setText(yGraph->plot()->name() + " --> COLOR MODE");
    infoModeLabel->setColor(yGraph->plot()->pen().color());
    QFontMetricsF fm(infoModeLabel->font());
    qreal pixelsWide = fm.width(infoModeLabel->text());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    QColor color = QColorDialog::getColor(Qt::white, nullptr, "plot color");
    if (color.isValid()) {
//        qDebug() << "doMenuPlotColorAction: " << color;
        QPen pen = yGraph->plot()->pen();
        pen.setColor(color);
        yGraph->plot()->setPen(pen);
        yGraph->info()->setColor(color);
        yGraph->info()->setSelectedPen(QPen(color));
        yGraph->info()->setSelectedColor(color);
        yGraph->rLine()->setPen(QPen(pen.color()));
        ui->plot->replot();
    }
    resetMouseWheelState();
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotScaleAction() {
    Q_ASSERT(contextMenu);
    resetMouseWheelState();
    int plotId = contextMenu->property("id").toInt();
    yaspGraph* yGraph = graphs[plotId];
    Q_ASSERT(yGraph);
    workingGraph = yGraph;
    infoModeLabel->setText(yGraph->plot()->name() + " --> SCALE MODE");
    infoModeLabel->setColor(yGraph->plot()->pen().color());
    QFontMetricsF fm(infoModeLabel->font());
    qreal pixelsWide = fm.width(infoModeLabel->text());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
//    qDebug() << "doMenuPlotScaleAction: " << contextMenu->property("id");
    wheelState = wheelScalePlot;
    startScalePlot = true;
    ui->plot->setInteraction(QCP::iRangeZoom, false);
}

/******************************************************************************************************************/
void MainWindow::doMenuPlotShowHideAction() {
    Q_ASSERT(contextMenu);
    resetMouseWheelState();
    int plotId = contextMenu->property("id").toInt();
    yaspGraph* yGraph = graphs[plotId];
    Q_ASSERT(yGraph);
    if (yGraph->plot()->visible()) {
        yGraph->plot()->setVisible(false);
        yGraph->rLine()->setVisible(false);
        plotShowHideAction->setText("show");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
        infoModeLabel->setText(yGraph->plot()->name() + " --> HIDE MODE");
        infoModeLabel->setColor(yGraph->plot()->pen().color());
    } else {
        yGraph->plot()->setVisible(true);
        yGraph->rLine()->setVisible(true);
        plotShowHideAction->setText("Hide");
        plotShowHideAction->setIcon(QIcon(":/Icons/Icons/icons8-hide-48.png"));
        infoModeLabel->setText(yGraph->plot()->name() + " --> SHOW MODE");
        infoModeLabel->setColor(yGraph->plot()->pen().color());
    }
    QFontMetricsF fm(infoModeLabel->font());
    qreal pixelsWide = fm.width(infoModeLabel->text());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    ui->plot->replot();
}

//*******************************************************************************************/
void MainWindow::saveDataPlot(yaspGraph* yGraph) {
    QCPGraph* g = yGraph->plot();
    if (!g->visible()) return;
    infoModeLabel->setText(yGraph->plot()->name() + " --> SAVE MODE");
    infoModeLabel->setColor(yGraph->plot()->pen().color());
//    plotting = false;
    qDebug() << "saveDataPlot Graph Data: " << g->data();
//    QCPDataMap* map = g->data();
//    // Iterate over Plot Data
//    if (map->size()) {
//        int measureGraphId = getIdOfQCPGraph(g);
//        Q_ASSERT(measureGraphId >= 0);
//        Q_ASSERT(measureGraphId < 9);
//        qDebug() << "GRAPH ID ---> " << measureGraphId;
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
//            yGraph->save(streamData, pointTime, nop);
        }
       qDebug() << "Close Log DATA : " << logData->fileName();
       logData->close();
       delete logData;
       logData = nullptr;
//    }
//       plotting = true;
}

/******************************************************************************************************************/
void MainWindow::saveSelectedGraph() {
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
    if (tracer) {
//        double coordX = ui->plot->xAxis->pixelToCoord(pX);
//        // get tracer origin
//        tracer->setGraphKey(traceerStartKey);
//        tracer->updatePosition();
//        rubberOrigin = tracer->position->pixelPoint();
//        double startX = tracer->position->key();
//        double startY = tracer->position->value() / measureMult;
//        tracer->setGraphKey(coordX);
//        tracer->updatePosition();
//        if (rubberBand) {
//            QPointF pp = tracer->position->pixelPoint();
//            rubberBand->setGeometry(QRectF(rubberOrigin, pp).normalized().toRect());
//            rubberBand->repaint();
//        }
//        double endX = tracer->position->key();
//        double endY = tracer->position->value() / measureMult;
//        QString coordinates("X: %1 Y: %2 DELTAX: %3 ms --> DELTAY: %4 (Mult: %5 )");
//        coordinates = coordinates.arg(startX).arg(startY).arg(endX - startX).arg(endY - startY).arg(measureMult);
//        ui->statusBar->setStyleSheet("background-color: lightgreen;");
//        ui->statusBar->showMessage(coordinates);
//        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Prints coordinates of mouse pointer in status bar on mouse release */
/******************************************************************************************************************/
void MainWindow::onMouseMoveInPlot(QMouseEvent *event) {
    if (mouseState == mouseMove) {
        if (wheelState == wheelZoom) {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
        }
        if (measureInProgress == false) {
            double xx = ui->plot->xAxis->pixelToCoord(event->x());
            double yy = ui->plot->yAxis->pixelToCoord(event->y());
            QString coordinates("X: %1 Y: %2 Points:%3");
            coordinates = coordinates.arg(xx).arg(yy).arg(dataPointNumber);
            ui->statusBar->setStyleSheet("background-color: SkyBlue;");
            ui->statusBar->showMessage(coordinates);
        } else {
            updateTracer(event->pos().x());
        }
    } else {
        if (mouseState == mouseShift) {
            if (mouseButtonState == Qt::LeftButton) {
                ui->plot->setInteractions(QCP::iRangeZoom | QCP::iSelectItems );
                shiftPlot(event->y());
            }
        }
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event) {
    Q_UNUSED(event)
//    mousePressed = false;
    startShiftPlot = false;
    mouseButtonState = Qt::NoButton;
    ui->statusBar->showMessage("release");
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    if (plotting) {
//        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
    } else {
//        if (tracer) {
//            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
//            tracer->blockSignals(false);
//        } else {
//            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems  | QCP::iSelectAxes | QCP::iSelectPlottables);
//        }
    }
}

/******************************************************************************************************************/
void MainWindow::resetMouseWheelState() {
    qDebug() << "resetMouseWheelState";
    mouseState = mouseMove;
    wheelState = wheelZoom;
    startScalePlot = false;
    startShiftPlot = false;
    infoModeLabel->setText("");
    infoModeLabel->setColor(QColor(0, 255, 255));
    QFontMetricsF fm(infoModeLabel->font());
    qreal pixelsWide = fm.width(infoModeLabel->text());
    infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
}

/******************************************************************************************************************/
void MainWindow::selectionChangedByUserInPlot() {
//    qDebug() << "selectionChangedByUserInPlot";
//    qDebug() << "selectionChangedByUserInPlot ui->plot->selectedGraphs().size() : " << ui->plot->selectedGraphs().size();
//    qDebug() << "selectionChangedByUserInPlot ui->plot->selectedItems().size() : " << ui->plot->selectedItems().size();
//    qDebug() << "selectionChangedByUserInPlot ui->plot->selectedPlottables().size() : " << ui->plot->selectedPlottables().size();
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems );
}

/******************************************************************************************************************/
void MainWindow::onMouseWheelInPlot(QWheelEvent *event) {
    if (wheelState == wheelZoom) {
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
        ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
        if (mouseButtonState == Qt::RightButton) {
            if (plotting) {
                ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
                QPoint numDegrees = event->angleDelta();
                if (numDegrees.y() == 0) return;
                double inc = plotTimeInSeconds / 100;
                if (numDegrees.y() > 0) {
                    ui->spinDisplayTime->setValue(ui->spinDisplayTime->value() + inc);
                } else {
                    ui->spinDisplayTime->setValue(ui->spinDisplayTime->value() - inc);
                }
            } else {
                ui->plot->axisRect()->setRangeZoom(Qt::Horizontal);
            }
        }
        updateTracer(event->pos().x());
    } else if (wheelState == wheelScalePlot) {
        QPoint numDegrees = event->angleDelta();
        scalePlot(numDegrees.y());
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseDoubleClickInPlot(QMouseEvent* event) {
    if (tracer) {
        double coordX = ui->plot->xAxis->pixelToCoord(event->pos().x());
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
//        rubberOrigin = tracer->position->pixelPoint().toPoint();
        if (rubberBand) {
            delete rubberBand;
        }
        traceerStartKey = tracer->position->key();
        rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->plot);
        rubberBand->setGeometry(QRectF(rubberOrigin, QSize()).toRect());
        rubberBand->show();
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
void MainWindow::onMousePressInPlot(QMouseEvent *event) {
    qDebug() << "onMousePressInPlot " << event->button();
    mouseButtonState = event->button();
    startShiftPlot = true;

    if (tracer) {
        tracer->blockSignals(true);
        if (event->button() == Qt::MiddleButton) {
        }
    }
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    if (ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        qDebug() << "xAxis";
        ui->plot->axisRect()->setRangeDrag(ui->plot->xAxis->orientation());
    } else if (ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        ui->plot->axisRect()->setRangeDrag(ui->plot->yAxis->orientation());
        qDebug() << "yAxis";
    } else {
        qDebug() << "xAxis yAxis";
        ui->plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
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
void MainWindow::plotLabelSelected(bool b) {
//    qDebug() << "plotLabelSelected : " << b;
    resetMouseWheelState();
    if (b) {
        //Q_ASSERT(ui->plot->selectedItems().size() == 1);
        qDebug() << "plotLabelSelected : " << ui->plot->selectedItems().size();
        QCPAbstractItem* item = ui->plot->selectedItems().at(0);
        qDebug() << "plotLabelSelected : " << item << " property id " << item->property("id");
        contextMenu = new QMenu(this);
        connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuTriggered(QAction*)));
        QVariant plotId = item->property("id");
        selectedPlotId = plotId.toInt();
        contextMenu->setProperty("id", plotId);
        contextMenu->setAttribute(Qt::WA_TranslucentBackground);
        contextMenu->setStyleSheet("QMenu {border-radius:16px;}");
        yaspGraph* yGraph = graphs[plotId.toInt()];
        Q_ASSERT(yGraph);
        QLabel* menuTitle = new QLabel("plot " + yGraph->plot()->name(), contextMenu);
        menuTitle->setAlignment(Qt::AlignCenter);
        QString styleTitle = "color:" + yGraph->plot()->pen().color().name() + ";"
                + "background-color:" + bgColor.name() + ";"
                + "font:bold;"
                + "padding:8px;";
//                + "border-radius:8px;";
//        + "border-width:2px;border-style:solid;border-color:black;font:bold;padding:2px;";
        menuTitle->setStyleSheet(styleTitle);
        QWidgetAction *LabelAction= new QWidgetAction(contextMenu);
        LabelAction->setDefaultWidget(menuTitle);
        contextMenu->addAction(LabelAction);
        QPen pen = yGraph->plot()->pen();
        pen.setWidth(1);
        pen.setDashPattern(plotDashPattern);
        yGraph->plot()->setPen(pen);
        yGraph->info()->setSelectedPen(pen);
        yGraph->info()->setSelectedColor(pen.color());
        QAction* action = contextMenu->addAction("Color", this, SLOT(doMenuPlotColorAction()));
        action->setIcon(QIcon(":/Icons/Icons/icons8-paint-palette-48.png"));
        action = plotShowHideAction = contextMenu->addAction("Hide", this, SLOT(doMenuPlotShowHideAction()));
        if (yGraph->plot()->visible()) {
            plotShowHideAction->setText("Hide");
            infoModeLabel->setText(yGraph->plot()->name() + " --> SHOW MODE");
            infoModeLabel->setColor(yGraph->plot()->pen().color());
            action->setIcon(QIcon(":/Icons/Icons/icons8-hide-48.png"));
        } else {
            plotShowHideAction->setText("Show");
            infoModeLabel->setText(yGraph->plot()->name() + " --> HIDE MODE");
            infoModeLabel->setColor(yGraph->plot()->pen().color());
            action->setIcon(QIcon(":/Icons/Icons/icons8-eye-48.png"));
        }
        QFontMetricsF fm(infoModeLabel->font());
        qreal pixelsWide = fm.width(infoModeLabel->text());
        infoModeLabel->position->setCoords(ui->plot->geometry().width() - pixelsWide - 32, 16);
        action = contextMenu->addAction("Scale", this, SLOT(doMenuPlotScaleAction()));
        action->setIcon(QIcon(":/Icons/Icons/icons8-height-48.png"));
        action = contextMenu->addAction("Shift", this, SLOT(doMenuPlotShiftAction()));
        action->setIcon(QIcon(":/Icons/Icons/icons8-shift-48.png"));
        action = contextMenu->addAction("Save", this, SLOT(saveSelectedGraph()));
        action->setIcon(QIcon(":/Icons/Icons/icons8-save-48.png"));

        ui->plot->setContextMenuPolicy(Qt::CustomContextMenu);
    } else {
        if (contextMenu) {
            QVariant plotId = contextMenu->property("id");
            yaspGraph* yGraph = graphs[plotId.toInt()];
            Q_ASSERT(yGraph);
            QPen pen = yGraph->plot()->pen();
            pen.setWidth(1);
            pen.setStyle(Qt::SolidLine);
            yGraph->plot()->setPen(pen);
            yGraph->info()->setSelectedPen(Qt::NoPen);
        }
        delete contextMenu;
        contextMenu = nullptr;
        selectedPlotId = -1;
        ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
    }
}

/******************************************************************************************************************/
void MainWindow::xAxisRangeChanged(const QCPRange& range) {
//    qDebug() << "xAxisRangeChanged: " << oldRange << " -> " << newRange;
//    qDebug() << "xAxisRangeChanged: " << oldRange.upper - oldRange.lower << " -> " << newRange.upper - newRange.lower;
    double val = range.upper - range.lower;
    if (!qFuzzyCompare(val, ui->spinDisplayTime->value())) {
//        qDebug() << "fixedTicker->tickStep: " << val;
        ui->spinDisplayTime->setValue(val);
    }
}

