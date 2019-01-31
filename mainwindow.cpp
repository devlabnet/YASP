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

/******************************************************************************************************************/
/* Constructor */
/******************************************************************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected(false), plotting(false), dataPointNumber(0), numberOfAxes(1), STATE(WAIT_START), numberOfPoints(NUMBER_OF_POINTS_DEF)
{
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
//    // Legend
//    ui->plot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
//    ui->plot->legend->setVisible(true);
//    QFont legendFont = font();  // start out with MainWindow's font..
//    legendFont.setPointSize(9); // and make a bit smaller for legend
//    ui->plot->legend->setFont(legendFont);
//    ui->plot->legend->setBrush(QBrush(QColor(255,255,255,230)));

    ui->plot->setNotAntialiasedElements(QCP::aeAll);                                      // used for higher performance (see QCustomPlot real time example)
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->plot->xAxis->setTickLabelFont(font);
    ui->plot->yAxis->setTickLabelFont(font);
    ui->plot->legend->setFont(font);

    ui->plot->yAxis->setAutoTickStep(false);                                              // User can change tick step with a spin box
    ui->plot->yAxis->setTickStep(500);                                                    // Set initial tick step
    ui->plot->xAxis->setTickLabelColor(gridColor);                              // Tick labels color
    ui->plot->yAxis->setTickLabelColor(gridColor);                              // See QCustomPlot examples / styled demo
    ui->plot->xAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridPen(QPen(subGridColor, 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setSubGridPen(QPen(subGridColor, 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridVisible(true);
    ui->plot->yAxis->grid()->setSubGridVisible(true);
    ui->plot->xAxis->setBasePen(QPen(gridColor));
    ui->plot->yAxis->setBasePen(QPen(gridColor));
    ui->plot->xAxis->setTickPen(QPen(gridColor));
    ui->plot->yAxis->setTickPen(QPen(gridColor));
    ui->plot->xAxis->setSubTickPen(QPen(gridColor));
    ui->plot->yAxis->setSubTickPen(QPen(gridColor));
    ui->plot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    ui->plot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);


    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
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

    serialPort = nullptr;                                                                    // Set serial port pointer to NULL initially
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(replot()));                       // Connect update timer to replot slot
    ui->menuWidgets->menuAction()->setVisible(false);

    /* Also, just I want to show you how to choose the color separator.
        * To do this we need to use a class QPallete, for which you choose the background color.
        * */
    QPalette p;
    p.setColor(QPalette::Background, QColor(144, 238, 144));
    ui->splitter->setPalette(p);
    ui->tabWidget->setCurrentIndex(0);
    ui->spinPoints->setMinimum(SPIN_MIN_DEF);
    ui->spinPoints->setMaximum(SPIN_MAX_DEF);
    ui->spinPoints->setSingleStep(SPIN_STEP_DEF);
    ui->spinPoints->setValue(NUMBER_OF_POINTS_DEF);
    ui->autoScrollLabel->setStyleSheet("QLabel { color : DodgerBlue; }");
    ui->autoScrollLabel->setText("Auto Scroll OFF, To allow move cursor to the end or SELECT Button ---> ");


    //qDebug() << "QLocale::system() --> " << QLocale::system().;
    // Clear the terminal
    on_clearTermButton_clicked();
    plotTime.start();
}

/******************************************************************************************************************/
/* Destructor */
/******************************************************************************************************************/
MainWindow::~MainWindow() {
//    qDebug() << "MainWindow Destructor";
    if(serialPort != nullptr) delete serialPort;
    delete ui;
}
/******************************************************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event) {
//   qDebug() << "MainWindow closeEvent";
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
    if (plotsToolBox != nullptr) {
        for (int i = plotsVector.size() - 1; i >= 0; i--) {
            graphContainer* gc = plotsVector[i];
            if (gc != nullptr) {
                gc->clearData();
                gc->clearLabels();
                gc->setUsed(false);
                delete gc;
            }
            // Remove everything from the plot
            plotsToolBox->removeTab(gc->getTabPos());
        }
        plotsVector.clear();
        delete plotsToolBox;
        plotsToolBox = nullptr;
        delete bottomWidget;
    }
    ui->plot->hide();
}

/******************************************************************************************************************/
void MainWindow::updateGraphNops(bool resetDelta) {
    ui->plot->clearItems();
    for (int i = plotsVector.size() - 1; i >= 0; i--) {
        graphContainer* gc = plotsVector[i];
        if (gc != nullptr) {
            gc->updateGraphNop(numberOfPoints, resetDelta);
        }
    }
}

/******************************************************************************************************************/
int MainWindow::getIdOfQCPGraph(QCPGraph* g) {
    for (int i = plotsVector.size() - 1; i >= 0; i--) {
        graphContainer* gc = plotsVector[i];
        if (gc != nullptr) {
            if (gc->getGraph() == g) {
                return i;
            }
        }
    }
    return -1;
}

/******************************************************************************************************************/
void MainWindow::updateGraphParams(QColor plotBgColor) {
    for (int i = plotsVector.size() - 1; i >= 0; i--) {
        graphContainer* gc = plotsVector[i];
        if (gc != nullptr) {
            gc->updateGraphParams(plotBgColor);
        }
    }
}

/******************************************************************************************************************/
/* Setup the plot area */
/******************************************************************************************************************/
void MainWindow::setupPlot() {
    plotsToolBox = new QTabWidget();
    plotsToolBox->setTabPosition(QTabWidget::North);
    ui->verticalLayoutPlots->addWidget(plotsToolBox);
    plotsToolBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    bottomWidget = new QLabel();
    bottomWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    bottomWidget->setPixmap(QPixmap(":/Icons/Icons/logo_devlabnet_small.png"));
    bottomWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    ui->verticalLayoutPlots->addWidget(bottomWidget);
    ui->plot->show();
    addPlots();
}

/******************************************************************************************************************/
void MainWindow::addPlots() {
    for (int  tabInd = 0;  tabInd < 10; ++ tabInd) {
        ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
        ui->plot->xAxis->setRange(0, numberOfPoints);                                      // Set x axis range for specified number of points
        setAutoYRange(ui->plot->yAxis->range().size());
        QString plotStr = "Plot " + QString::number( tabInd);
        graphContainer* gc = new graphContainer(ui->plot->addGraph(), numberOfPoints, plotStr, colours[ tabInd],  tabInd);
        plotsVector.insert(tabInd, gc);
        connect(gc, SIGNAL(plotColorChanged(int, QColor)), this, SLOT(plotColorChanged(int, QColor)));
        gc->updateGraphParams(bgColor);
    }
}

/******************************************************************************************************************/
void MainWindow::plotColorChanged(int tabInd, QColor color) {
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    plotsToolBox->tabBar()->setTabIcon(tabInd, QIcon(pixmap));
    ui->plot->replot();
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
        setupPlot();                                                                          // Create the QCustomPlot area
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
    updateTimer.start(20);                                                                // Slot is refreshed 20 times per second
    connected = true;                                                                     // Set flags
    plotting = true;
    ui->tabWidget->setCurrentIndex(1);
    // Reset the Device via DTR
    serialPort->setDataTerminalReady(true);
    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    connect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));
    plotTime.restart();
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
        ui->plot->xAxis->setRange(dataPointNumber - numberOfPoints, dataPointNumber);
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Stop Plot Button */
/******************************************************************************************************************/
void MainWindow::on_stopPlotButton_clicked() {
    if (plotting) {                                                                        // Stop plotting
        updateTimer.stop();                                                               // Stop updating plot timer
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
        // setup policy and connect slot for context menu popup:
        ui->plot->setContextMenuPolicy(Qt::CustomContextMenu);
    } else {                                                                              // Start plotting
        updateTimer.start();                                                              // Start updating plot timer
        plotting = true;
        ui->stopPlotButton->setText("Stop Plot");
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        ui->plot->setContextMenuPolicy(Qt::PreventContextMenu);
        cancelMeasure();
    }
}

/******************************************************************************************************************/
bool MainWindow::isColor(QString str) {
    if (!str.isEmpty()) {
        // check if start with #, if yes --> color
        if (str.at(0) == "#") {
            str = str.remove(0, 1);
            QRegularExpression hexMatcher("^[0-9A-F]{6}$",
                                          QRegularExpression::CaseInsensitiveOption);
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
void MainWindow::onNewPlotDataArrived(QStringList newData) {
    if (newData.size() > 1) {
        int plotId = newData.at(0).toInt();
        if ((plotId < 0) || (plotId > 9)) {
            qDebug() << plotTime.elapsed() << " BAD PLOT ID : " << plotId << " --> " << newData;
            addMessageText(QString::number(plotTime.elapsed()) + " BAD PLOT ID : " + QString::number(plotId) + " --> " + newData.join(" / "), "tomato");
            return;
        }
//        qDebug() << "PLOT DATA : " << plotId << " --> " << newData;
        graphContainer* plot = plotsVector[plotId];
        Q_ASSERT(plot != nullptr);
        int tabPos;
        if (!plot->isUsed()) {
            tabPos = plotsToolBox->addTab(plot, plot->getName());
            plotsToolBox->setTabEnabled(tabPos, true);
            plot->setColor(colours[plotId]);
            plot->setUsed(true);
            plot->setTabPos(tabPos);
        } else {
            tabPos = plot->getTabPos();
        }
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
                plot->setColor(QColor(param1));
        } else {
            if ((!param1.isEmpty()) && (plot->getName() != param1)) {
                plot->setName(param1);
                plotsToolBox->tabBar()->setTabText(tabPos, param1);
            }
        }
        if (isColor(param2)) {
            plot->setColor(QColor(param2));
        } else {
            if ((!param2.isEmpty()) && (plot->getName() != param2)) {
                plot->setName(param2);
                plotsToolBox->tabBar()->setTabText(tabPos, param2);
            }
        }
    }
}

/******************************************************************************************************************/
/* Slot for new data from serial port . Data is comming in QStringList and needs to be parsed */
/******************************************************************************************************************/
void MainWindow::onNewDataArrived(QStringList newData) {
    Q_ASSERT(newData.size() > 0);
    int plotId = newData.at(0).toInt();
    if ((plotId < 0) || (plotId > 9)) {
        addMessageText(QString::number(plotTime.elapsed()) + " BAD DATA ID : " + QString::number(plotId) + " --> " + newData.join(" / "), "tomato");
        return;
    }
    if(plotting) {
        int dataListSize = newData.size();                                                    // Get size of received list
        dataPointNumber++;
        if (dataListSize > 1) {
            double val = newData[1].toDouble();
            // Add data to graphs according plot Id
            graphContainer* plot = plotsVector[plotId];
            Q_ASSERT(plot != nullptr);
            int tabPos;
            if (!plot->isUsed()) {
                tabPos = plotsToolBox->addTab(plot, plot->getName());
                plotsToolBox->setTabEnabled(tabPos, true);
                plot->setColor(colours[plotId]);
                plot->setUsed(true);
                plot->setTabPos(tabPos);
            } else {
                tabPos = plot->getTabPos();
            }
            int time = plotTime.elapsed();
            plot->addData(dataPointNumber, val, time);
            if (logFile != nullptr) {
                if (plot->isDisplayed()) {
                    streamLog << plot->getName() << ";" << dataPointNumber << ";" << val << ";" << time << "\n";
                }
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
    return isdigit( ucc) || isspace( ucc) || ( ucc == '-') || ( ucc == '.');
}

/******************************************************************************************************************/
/* Read data for inside serial port */
/******************************************************************************************************************/
void MainWindow::readData() {
    if (serialPort->bytesAvailable()) {                                                    // If any bytes are available
        data = serialPort->readAll();         // Read all data in QByteArray
        if (!updateTimer.isActive()) return;
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
                        QStringList incomingData = receivedData.split(' ');               // Split string received from port and put it into list
                        emit newData(incomingData);                                       // Emit signal for data received with the list
                        break;
                    } else if (checkEndMsgMissed(cc)) {
                        break;
                    } else if (isNumericChar(cc)) {            // If examined char is a digit, and not '$' or ';', append it to temporary string
                        receivedData.append( cc);
                    }
                    break;
                case IN_PLOT_MSG:                                                          // If state is IN_MESSAGE
                    if( cc == END_MSG) {                                              // If char examined is ;, switch state to END_MSG
                        STATE = WAIT_START;
                        QStringList incomingData = receivedData.split(' ');               // Split string received from port and put it into list
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
void MainWindow::setAutoYRange(double r, bool resetDelta) {
    int step = 10;
    if (r < step) {
        ui->plot->yAxis->setTickStep(1);
        return;
    }
    int m = r / step;
    int v = qMax((int)(log10(m)), 1);
    int mult =  pow(10, v);
    int x = m / mult;
    int vMin = qMax(x * mult, 5);
    ui->plot->yAxis->setTickStep(vMin);
    updateGraphNops(resetDelta);
    ui->plot->replot();
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
    numberOfPoints = NUMBER_OF_POINTS_DEF;
    ui->plot->yAxis->setRange(-DEF_YAXIS_RANGE, DEF_YAXIS_RANGE);       // Set lower and upper plot range
    ui->plot->xAxis->setRange(dataPointNumber - numberOfPoints, dataPointNumber);
    setAutoYRange(ui->plot->yAxis->range().size(), true);
    on_spinPoints_valueChanged(numberOfPoints);
}

/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinPoints_valueChanged(int arg1) {
    numberOfPoints = arg1;
    updateGraphNops();
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
        int measureGraphId = getIdOfQCPGraph(gr);
        Q_ASSERT(measureGraphId >= 0);
        Q_ASSERT(measureGraphId < 9);
        qDebug() << "GRAPH ID ---> " << measureGraphId;
        measureMult = plotsVector[measureGraphId]->getMult();
        tracer->setGraph(gr);
        ui->plot->deselectAll();
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
void MainWindow::cancelMeasure() {
    cleanTracer();
    ui->plot->setCursor(Qt::ArrowCursor);
    ui->plot->deselectAll();
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::saveDataPlot(QCPGraph* g) {
    qDebug() << "Graph Data: " << g->data();
    QCPDataMap* map = g->data();
    // Iterate over Plot Data
    if (map->size()) {
        int measureGraphId = getIdOfQCPGraph(g);
        Q_ASSERT(measureGraphId >= 0);
        Q_ASSERT(measureGraphId < 9);
        qDebug() << "GRAPH ID ---> " << measureGraphId;
        QString plotName = plotsVector[measureGraphId]->getName();
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
            QLocale locale = QLocale("fr_FR");
            locale.setNumberOptions(QLocale::OmitGroupSeparator);
            //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            streamData.setLocale(locale);

            // Set Headers
//            streamData << "NAME" << ";" << "TIME" << ";" << "VALUE" << "\n";
            streamData << "TIME" << ";" << "VALUE" << "\n";
            QCPDataMap::const_iterator it = map->constBegin();
            while (it != map->constEnd()) {
                QCPData data =  it.value();
//                streamData << plotName << ";" << data.key << ";" << data.value  << "\n";
                streamData << data.key << ";" << data.value  << "\n";
                ++it;
            }
        }
       qDebug() << "Close Log DATA : " << logData->fileName();
       logData->close();
       delete logData;
       logData = nullptr;
    }
}

/******************************************************************************************************************/
void MainWindow::saveSelectedGraph() {
    QList<QCPGraph*> sg = ui->plot->selectedGraphs();
    qDebug() << "Selected Graph: " << sg;
    QCPGraph* g = sg.at(0);
    if (!g->visible()) return;
    saveDataPlot(g);
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
        double coordX = ui->plot->xAxis->pixelToCoord(pX);
        // get tracer origin
        tracer->setGraphKey(traceerStartKey);
        tracer->updatePosition();
        rubberOrigin = tracer->position->pixelPoint();
        double startX = tracer->position->key();
        double startY = tracer->position->value() / measureMult;
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
        if (rubberBand) {
            QPointF pp = tracer->position->pixelPoint();
            rubberBand->setGeometry(QRectF(rubberOrigin, pp).normalized().toRect());
            rubberBand->repaint();
        }
        double endX = tracer->position->key();
        double endY = tracer->position->value() / measureMult;
        QString coordinates("X: %1 Y: %2 DELTAX: %3 ms --> DELTAY: %4 (Mult: %5 )");
        coordinates = coordinates.arg(startX).arg(startY).arg(endX - startX).arg(endY - startY).arg(measureMult);
        ui->statusBar->setStyleSheet("background-color: lightgreen;");
        ui->statusBar->showMessage(coordinates);
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Prints coordinates of mouse pointer in status bar on mouse release */
/******************************************************************************************************************/
void MainWindow::onMouseMoveInPlot(QMouseEvent *event) {
    if (measureInProgress == false) {
        double xx = ui->plot->xAxis->pixelToCoord(event->x());
        double yy = ui->plot->yAxis->pixelToCoord(event->y());
        QString coordinates("X: %1 Y: %2");
        coordinates = coordinates.arg(xx).arg(yy);
        ui->statusBar->setStyleSheet("background-color: SkyBlue;");
        ui->statusBar->showMessage(coordinates);
    } else {
        updateTracer(event->pos().x());
    }
}

/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event) {
    Q_UNUSED(event)
    mousePressed = false;
    ui->statusBar->showMessage("release");
    if (plotting) {
        ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    } else {
        if (tracer) {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            tracer->blockSignals(false);
        } else {
            ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
        }
    }
}

/******************************************************************************************************************/
void MainWindow::selectionChangedByUserInPlot() {
    qDebug() << "selectionChangedByUserInPlot";
}

/******************************************************************************************************************/
void MainWindow::onMouseWheelInPlot(QWheelEvent *event) {
    Q_UNUSED(event)
    setAutoYRange(ui->plot->yAxis->range().size());
    updateTracer(event->pos().x());
}

/******************************************************************************************************************/
void MainWindow::onMouseDoubleClickInPlot(QMouseEvent* event) {
    if (tracer) {
        double coordX = ui->plot->xAxis->pixelToCoord(event->pos().x());
        tracer->setGraphKey(coordX);
        tracer->updatePosition();
        rubberOrigin = tracer->position->pixelPoint().toPoint();
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
    mousePressed = true;
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
    QMenu* contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    if ((ui->plot->selectedGraphs().size() > 0) || (measureInProgress)) {
        if (measureInProgress) {
            contextMenu->addAction("Stop Measure", this, SLOT(cancelMeasure()));
        } else {
            contextMenu->addAction("Save Graph Data", this, SLOT(saveSelectedGraph()));
            contextMenu->addAction("Start Measure", this, SLOT(doMeasure()));
        }
    } else {
        if (ui->plot->graphCount() > 0) {
            //contextMenu->addAction("Save All Graphs Data", this, SLOT(saveAllGraphs()));
        }
    }
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
    ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");
    ui->plot->setBackground(QBrush(bgColor));   // Background for the plot area
    updateGraphParams(bgColor);
    if (tracer) {
        if (bgColor.lightness() > 128) {
            tracer->setPen(QPen(Qt::black));
        } else {
            tracer->setPen(QPen(Qt::white));
        }
    }
    ui->plot->replot();
}

/******************************************************************************************************************/
void MainWindow::on_plotsInfoRadio_clicked(bool checked) {
    for (int var = 0; var < plotsVector.size(); ++var) {
        plotsVector[var]->setRadioInfo(checked);
    }
    ui->plot->replot();
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
