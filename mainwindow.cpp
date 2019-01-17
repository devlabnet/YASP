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
#include "graphcontainer.h"

/******************************************************************************************************************/
/* Constructor */
/******************************************************************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected(false), plotting(false), dataPointNumber(0), numberOfAxes(1), STATE(WAIT_START), NUMBER_OF_POINTS(500)
{
    ui->setupUi(this);
    createUI();                                                                           // Create the UI
    QColor gridColor = QColor(170,170,170);
    //ui->bgColorButton.
    ui->bgColorButton->setAutoFillBackground(true);
    ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");

    QColor subGridColor = QColor(80,80,80);
    ui->plot->setBackground(QBrush(bgColor));                                    // Background for the plot area
//    plotsToolBox->removeItem(0);
    ui->plot->hide();
    ui->stopPlotButton->setEnabled(false);                                                // Plot button is disabled initially

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
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);


    // Slot for printing coordinates
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMoveInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseReleaseInPlot(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onMouseWheelInPlot(QWheelEvent*)));

    serialPort = nullptr;                                                                    // Set serial port pointer to NULL initially
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(replot()));                       // Connect update timer to replot slot
    ui->menuWidgets->menuAction()->setVisible(false);
//    ui->splitter->setStretchFactor(0,1);
//    ui->splitter->setStretchFactor(1,0);

    /* Also, just I want to show you how to choose the color separator.
        * To do this we need to use a class QPallete, for which you choose the background color.
        * */
    QPalette p;
    p.setColor(QPalette::Background, QColor(144, 238, 144));
    /* And sets the palette QSplitter
        * */
    ui->splitter->setPalette(p);
    // Clear the terminal
    on_clearTermButton_clicked();

}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Destructor */
/******************************************************************************************************************/
MainWindow::~MainWindow()
{
//    qDebug() << "MainWindows Destructor";
    if(serialPort != nullptr) delete serialPort;
//    widgets.hide();
//    widgets.deleteLater();
    delete ui;
}
/******************************************************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
//    qDebug() << "MainWindows closeEvent";
   // or event->accept(); but fine 'moments' are there
   QMainWindow::closeEvent(event);
}


/******************************************************************************************************************/
/**Create the GUI */
/******************************************************************************************************************/
void MainWindow::createUI()
{
    if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
        enableControls(false);
        ui->connectButton->setEnabled(false);
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

    ui->comboAxes->addItem("1");                                                          // Populate axes combo box; 3 axes maximum allowed
    ui->comboAxes->addItem("2");
    ui->comboAxes->addItem("3");
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Enable/disable controls */
/******************************************************************************************************************/
void MainWindow::enableControls(bool enable)
{
    ui->comboBaud->setEnabled(enable);                                                    // Disable controls in the GUI
    ui->comboData->setEnabled(enable);
    ui->comboParity->setEnabled(enable);
    ui->comboPort->setEnabled(enable);
    ui->comboStop->setEnabled(enable);
    ui->comboAxes->setEnabled(enable);
}
/******************************************************************************************************************/

/******************************************************************************************************************/
void MainWindow::cleanGraphs() {
    ui->plot->clearItems();
    if (plotsToolBox != nullptr) {
        for (int i = plotsToolBox->count(); i >= 0; i--) {
            graphContainer* gc = dynamic_cast<graphContainer*>(plotsToolBox->widget(i));
//            qDebug() << "graphContainer: " << gc;
            if (gc != nullptr) {
                gc->clearData();
                gc->clearLabels();
//                qDebug() << "delete widget: " << gc;
                delete gc;
            }
            // Remove everything from the plot
            plotsToolBox->removeItem(i);
        }
        delete plotsToolBox;
        plotsToolBox = nullptr;
    }
//    plotsToolBox->hide();
    ui->plot->hide();
}

/******************************************************************************************************************/
void MainWindow::updateGraphs() {
    ui->plot->clearItems();
    for (int i = plotsToolBox->count(); i >= 0; i--) {
        graphContainer* gc = dynamic_cast<graphContainer*>(plotsToolBox->widget(i));
        if (gc != nullptr) {
            gc->updateGraph(NUMBER_OF_POINTS);
        }
    }
}

/******************************************************************************************************************/
/* Setup the plot area */
/******************************************************************************************************************/
void MainWindow::setupPlot()
{
    //ui->verticalLayoutPlots->addWidget(new QPushButton("0"));
    cleanGraphs();
    plotsToolBox = new QToolBox();
    ui->verticalLayoutPlots->addWidget(plotsToolBox);
    plotsToolBox->setMinimumSize(400,200);
    plotsToolBox->setMaximumWidth(400);
    //ui->verticalLayoutPlots->addWidget(new QPushButton("1"));
    ui->plot->show();
    addPlots();
}
/******************************************************************************************************************/

void MainWindow::addPlots() {
    for (int var = 0; var < 10; ++var) {
        ui->plot->yAxis->setRange(-1000, 1000);       // Set lower and upper plot range
        ui->plot->xAxis->setRange(0, NUMBER_OF_POINTS);                                      // Set x axis range for specified number of points
        setAutoYRange(ui->plot->yAxis->range().size());
    //ui->plot->xAxis->setTickLabels(false);
    //    ui->plot->xAxis->setTickStep(25.0);
        QString plotStr = "Plot " + QString::number(var);
        plotsToolBox->insertItem(var, new graphContainer(ui->plot->addGraph(), NUMBER_OF_POINTS, plotStr, colours[var], var), plotStr);

    }
//    //    ui->plot->yAxis->setTickStep(ui->spinYStep->value());                                // Set tick step according to user spin box
//        numberOfAxes = ui->comboAxes->currentText().toInt();                                 // Get number of axes from the user combo
//        ui->plot->yAxis->setRange(-1000, 1000);       // Set lower and upper plot range
//        ui->plot->xAxis->setRange(0, NUMBER_OF_POINTS);                                      // Set x axis range for specified number of points
//        setAutoYRange(ui->plot->yAxis->range().size());
//    //ui->plot->xAxis->setTickLabels(false);
//    //    ui->plot->xAxis->setTickStep(25.0);
//        QString plotStr = "Plot 0";
//        if(numberOfAxes == 1) {                                                               // If 1 axis selected
//                                                               // add Graph 0
//    //        ui->plot->graph(0)->setPen(QPen(Qt::red));
//            //plotsToolBox->setItemText(0, "Plot 1");
//            plotsToolBox->insertItem(0, new graphContainer(ui->plot->addGraph(), NUMBER_OF_POINTS, plotStr, 0), plotStr);
//        } else if(numberOfAxes == 2) {                                                        // If 2 axes selected
//            plotsToolBox->insertItem(0, new graphContainer(ui->plot->addGraph(), NUMBER_OF_POINTS, plotStr, 0), plotStr);
//            plotStr = "Plot 1";
//            plotsToolBox->insertItem(1, new graphContainer(ui->plot->addGraph(), NUMBER_OF_POINTS, plotStr, 1), plotStr);
//    //        ui->plot->addGraph();                                                             // add Graph 0
//    //        ui->plot->graph(0)->setPen(QPen(Qt::red));
//    //        ui->plot->addGraph();                                                             // add Graph 1
//    //        ui->plot->graph(1)->setPen(QPen(Qt::yellow));
//        }
//    //    else if(numberOfAxes == 3) {                                                        // If 3 axis selected
//    //        ui->plot->addGraph();                                                             // add Graph 0
//    //        ui->plot->graph(0)->setPen(QPen(Qt::red));
//    //        ui->plot->addGraph();                                                             // add Graph 1
//    //        ui->plot->graph(1)->setPen(QPen(Qt::yellow));
//    //        ui->plot->addGraph();                                                             // add Graph 2
//    //        ui->plot->graph(2)->setPen(QPen(Qt::green));
//    //    }

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
        receivedData.clear();
        noMsgReceivedData.clear();
        serialPort->clear(QSerialPort::AllDirections);
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
//            while(serialPort->waitForReadyRead(0));
//            {
//                qDebug() << "t";
//                serialPort->clear(QSerialPort::AllDirections);
//            }

        portOpenedSuccess();
    } else {
        ui->statusBar->showMessage("Cannot open port!");
        qDebug() << serialPort->errorString();
    }

}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Port Combo Box index changed slot; displays info for selected port when combo box is changed */
/******************************************************************************************************************/
void MainWindow::on_comboPort_currentIndexChanged(const QString &arg1)
{
    QSerialPortInfo selectedPort(arg1);                                                   // Dislplay info for selected port
    ui->statusBar->showMessage(selectedPort.description());
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Connect Button clicked slot; handles connect and disconnect */
/******************************************************************************************************************/
void MainWindow::on_connectButton_clicked()
{
    if (connected) {
        closePort();
    } else {                                                                              // If application is not connected, connect
        openPort();                         // Open serial port and connect its signals
    }
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Slot for port opened successfully */
/******************************************************************************************************************/
void MainWindow::portOpenedSuccess()
{
    serialPort->setDataTerminalReady(false);
    ui->menuWidgets->menuAction()->setVisible(true);
    if ((widgets != nullptr)) {
        widgets->setSerialPort(serialPort);
    }
    //qDebug() << "Port opened signal received!";
    ui->connectButton->setText("Disconnect");                                             // Change buttons
    ui->statusBar->showMessage("Connected!");
    enableControls(false);                                                                // Disable controls if port is open
    ui->stopPlotButton->setText("Stop Plot");                                             // Enable button for stopping plot
    ui->stopPlotButton->setEnabled(true);
    ui->saveJPGButton->setEnabled(true);                                                  // Enable button for saving plot
    setupPlot();                                                                          // Create the QCustomPlot area
    updateTimer.start(20);                                                                // Slot is refreshed 20 times per second
    connected = true;                                                                     // Set flags
    plotting = true;
    ui->tabWidget->setCurrentIndex(1);
    serialPort->setDataTerminalReady(true);

    //connect(cmdSerial,SIGNAL(readyRead()),this,SLOT(onDataReadyRead()));

    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    connect(this, SIGNAL(newPlotData(QStringList)), this, SLOT(onNewPlotDataArrived(QStringList)));

}

/******************************************************************************************************************/
void MainWindow:: closePort() {
    cleanGraphs();
    // Clear the terminal
    on_clearTermButton_clicked();
    //qDebug() << "Port closed signal received!";
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
void MainWindow::replot()
{
    if(connected) {
        ui->plot->xAxis->setRange(dataPointNumber - NUMBER_OF_POINTS, dataPointNumber);
        ui->plot->replot();
    }
}

/******************************************************************************************************************/
/* Stop Plot Button */
/******************************************************************************************************************/
void MainWindow::on_stopPlotButton_clicked()
{
    if(plotting) {                                                                        // Stop plotting
        updateTimer.stop();                                                               // Stop updating plot timer
        plotting = false;
        ui->stopPlotButton->setText("Start Plot");
    } else {                                                                              // Start plotting
        updateTimer.start();                                                              // Start updating plot timer
        plotting = true;
        ui->stopPlotButton->setText("Stop Plot");
    }
}

/******************************************************************************************************************/
bool MainWindow::isColor(QString str) {
//    qDebug() << "isColor: " << str;
    if (!str.isEmpty()) {
        // check if start with #, if yes --> color
        if (str.at(0) == "#") {
            str = str.remove(0, 1);
//            qDebug() << " --> isColor: " << str;
            QRegularExpression hexMatcher("^[0-9A-F]{6}$",
                                          QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = hexMatcher.match(str);
            if (match.hasMatch()) {
                // Found hex string of length 6.
//                qDebug() << " --> isColor OK ";
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
            qDebug() << "BAD PLOT ID : " << plotId << " --> " << newData;
            return;
        }
        graphContainer* plot = dynamic_cast<graphContainer*>(plotsToolBox->widget(plotId));
        Q_ASSERT(plot != nullptr);

//        qDebug() << "Plot: " << newData;
        QString param1 = "";
        QString param2 = "";
        QString id = newData.at(0);
        if (newData.size() > 1) {
            if (newData.size() > 2) {
                param2 = newData.at(2);
            }
            param1 = newData.at(1);
        }
//        qDebug() << "Plot Id : " << id;
//        qDebug() << "Param1 : " << param1;
//        qDebug() << "Param2 : " << param2;
        if (isColor(param1)) {
                plot->setColor(QColor(param1));
        } else {
            if ((!param1.isEmpty()) && (plot->getName() != param1)) {
                plot->setName(param1);
            }
        }
        if (isColor(param2)) {
            plot->setColor(QColor(param2));
        } else {
            if ((!param2.isEmpty()) && (plot->getName() != param2)) {
                plot->setName(param2);
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
        qDebug() << "BAD DATA ID : " << plotId << " --> " << newData;
        return;
    }
    if(plotting) {
        int dataListSize = newData.size();                                                    // Get size of received list
        dataPointNumber++;
        if (dataListSize > 1) {
            double val = newData[1].toDouble();
            // Add data to graphs according plot Id
            graphContainer* plot = dynamic_cast<graphContainer*>(plotsToolBox->widget(plotId));
            if (plot != nullptr) {
                plot->addData(dataPointNumber, val);
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
    ui->receiveTerminal->moveCursor (QTextCursor::End);
    QString html = "<span style=\"color:"+color+";font-weight:bold\">"+string+"</span>";
    ui->receiveTerminal->appendHtml(html);
    ui->receiveTerminal->moveCursor (QTextCursor::End);
}

/******************************************************************************************************************/
bool MainWindow::checkEndMsgMissed(unsigned char cc) {
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

//        for (int var = 0; var < 4; ++var) {
//            graphContainer* plot = dynamic_cast<graphContainer*>(plotsToolBox->widget(var));
//            Q_ASSERT(plot != nullptr);
//            plot->setColor(QColor("black"));
//        }
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
/* Read data for inside serial port */
/******************************************************************************************************************/
void MainWindow::readData() {
    if (serialPort->bytesAvailable()) {                                                    // If any bytes are available
        data = serialPort->readAll();         // Read all data in QByteArray
        if(!data.isEmpty()) {                                                             // If the byte array is not empty
            char *temp = data.data();
            // Get a '\0'-terminated char* to the data
            for(int i = 0; temp[i] != '\0'; i++) {                                        // Iterate over the char*
                unsigned char cc = temp[i];
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
                            //                            qDebug() << " = " << cc << " / "  << temp[i];
                            if (!noMsgReceivedData.isEmpty()) {
                                addMessageText(noMsgReceivedData, "orange");
                                //                                qDebug() <<  noMsgReceivedData;
                            }
                            noMsgReceivedData.clear();
                        } else {
                            if ( cc != '\r') {
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
                    } else if (isdigit( cc) || isspace( cc) || ( cc == '-') || ( cc == '.') ) {            // If examined char is a digit, and not '$' or ';', append it to temporary string
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

void MainWindow::setAutoYRange(double r) {
    int step = 10;
    if (r < step) {
//        ui->spinYStep->setMinimum(1);
//        ui->spinYStep->setMaximum(50);
//        ui->spinYStep->setValue(1);
        ui->plot->yAxis->setTickStep(1);
        return;
    }
    int m = r / step;
    int v = qMax((int)(log10(m)), 1);
    int mult =  pow(10, v);
    int x = m / mult;
//    qDebug() <<  r << " -> " << m  << " -> " <<  v << " -> " << x;
    int vMin = qMax(x * mult, 5);
//    int vMax = vMin * step;
//    ui->spinYStep->setMinimum(vMin);
//    ui->spinYStep->setMaximum(vMax);
//    ui->spinYStep->setValue(vMin);
    ui->plot->yAxis->setTickStep(vMin);
//    qDebug() <<  vMin << " -> " << vMax;
}
/******************************************************************************************************************/
/* Number of axes combo; when changed, display axes colors in status bar */
/******************************************************************************************************************/
void MainWindow::on_comboAxes_currentIndexChanged(int index)
{
    if(index == 0) {
        ui->statusBar->showMessage("Axis 1: Red");
    } else if(index == 1) {
        ui->statusBar->showMessage("Axis 1: Red; Axis 2: Yellow");
    } else {
        ui->statusBar->showMessage("Axis 1: Red; Axis 2: Yellow; Axis 3: Green");
    }
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Spin box for changing the Y Tick step */
/******************************************************************************************************************/
//void MainWindow::on_spinYStep_valueChanged(int arg1)
//{
//    ui->plot->yAxis->setTickStep(arg1);
//    ui->plot->replot();
//}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Save a JPG image of the plot to current EXE directory */
/******************************************************************************************************************/
void MainWindow::on_saveJPGButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot"),
                               "",
                               tr("Images (*.jpg)"));

    qDebug() << "Save JPEG : " << fileName;
//    ui->plot->saveJpg(QString::number(dataPointNumber) + ".jpg");
    ui->plot->saveJpg(fileName);
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Reset the zoom of the plot to the initial values */
/******************************************************************************************************************/
void MainWindow::on_resetPlotButton_clicked()
{
    ui->plot->yAxis->setRange(0, 4095);
    ui->plot->xAxis->setRange(dataPointNumber - NUMBER_OF_POINTS, dataPointNumber);
    ui->plot->yAxis->setTickStep(500);
    ui->plot->replot();
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Prints coordinates of mouse pointer in status bar on mouse release */
/******************************************************************************************************************/
void MainWindow::onMouseMoveInPlot(QMouseEvent *event)
{
    int xx = static_cast<int>(ui->plot->xAxis->pixelToCoord(event->x()));
    int yy = static_cast<int>(ui->plot->xAxis->pixelToCoord(event->y()));
    //int yy = ui->plot->yAxis->pixelToCoord(event->y());
    QString coordinates("X: %1 Y: %2");
    coordinates = coordinates.arg(xx).arg(yy);
    ui->statusBar->showMessage(coordinates);
}
/******************************************************************************************************************/
void MainWindow::onMouseReleaseInPlot(QMouseEvent *event)
{
    Q_UNUSED(event)
//    int xx = static_cast<int>(ui->plot->xAxis->pixelToCoord(event->x()));
//    int yy = static_cast<int>(ui->plot->xAxis->pixelToCoord(event->y()));
//    //int yy = ui->plot->yAxis->pixelToCoord(event->y());
//    QString coordinates("X: %1 Y: %2");
//    coordinates = coordinates.arg(xx).arg(yy);
    ui->statusBar->showMessage("release");
}

void MainWindow::onMouseWheelInPlot(QWheelEvent *event)
{
    Q_UNUSED(event)
    setAutoYRange(ui->plot->yAxis->range().size());
}


/******************************************************************************************************************/
/* Spin box controls how many data points are collected and displayed */
/******************************************************************************************************************/
void MainWindow::on_spinPoints_valueChanged(int arg1)
{
    NUMBER_OF_POINTS = arg1;
    updateGraphs();
    ui->plot->replot();
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Shows a window with instructions */
/******************************************************************************************************************/
void MainWindow::on_actionHow_to_use_triggered()
{
    if (helpWindow == nullptr) {
        helpWindow = new HelpWindow(this);
        helpWindow->setWindowTitle("How to use this application");
        helpWindow->show();
    }
}
/******************************************************************************************************************/

void MainWindow::on_sendLine_returnPressed()
{

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

void MainWindow::on_clearTermButton_clicked()
{
    ui->receiveTerminal->clear();
    ui->receiveTerminal->moveCursor (QTextCursor::End);
}

void MainWindow::on_actionShowWidgets_triggered()
{
    if (widgets == nullptr) {
        widgets = new DialogWidgets(serialPort, this);
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

void MainWindow::on_bgColorButton_pressed()
{
    bgColor = QColorDialog::getColor(bgColor, this, "Select Background Color");
    ui->bgColorButton->setStyleSheet("background-color:" + bgColor.name() + "; color: rgb(0, 0, 0)");
    ui->plot->setBackground(QBrush(bgColor));   // Background for the plot area
}
