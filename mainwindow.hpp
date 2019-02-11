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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <QSerialPortInfo>
#include "helpwindow.hpp"
#include "dialogwidgets.h"
#include <QToolBox>
//#include "tabwidget.h"
#include <QTextEdit>
#include "graphcontainer.h"
#include <QTime>
#include <QRubberBand>
#include "yaspgraph.h"

#define START_MSG      0x10
#define PLOT_MSG       0X11
#define END_MSG        0X12
#define SPACE_MSG      0X13

#define WAIT_START      1
#define IN_MESSAGE      2
#define IN_PLOT_MSG     3
#define UNDEFINED       4

#define NUMBER_OF_POINTS_DEF 10000
#define DEF_YAXIS_RANGE 1500
#define SPIN_MIN_DEF 100
#define SPIN_MAX_DEF 100000
#define SPIN_STEP_DEF 1000

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void replot();
    void addTickX();
    // Slot for repainting the plot
    void onNewDataArrived(QStringList newData);                                           // Slot for new data from serial port
    void onNewPlotDataArrived(QStringList newData);                                       // Slot for new data from serial port
    void readData();                                                                      // Slot for inside serial port
    void plotColorChanged(int tabInd, QColor color);
    void dataTerminalReadyChanged(bool dtr);
    void messageSent(QString str);
    void on_comboPort_currentIndexChanged(const QString &arg1);                           // Slot displays message on status bar
    void on_connectButton_clicked();                                                      // Manages connect/disconnect
    void on_stopPlotButton_clicked();                                                     // Starts and stops plotting
    void on_saveJPGButton_clicked();                                                      // Button for saving JPG
    void on_resetPlotButton_clicked();                                                    // Resets plot to initial zoom and coordinates
    void onMouseMoveInPlot(QMouseEvent *event);                                           // Displays coordinates of mouse pointer when clicked in plot in status bar
    void onMouseReleaseInPlot(QMouseEvent *event);
    void onMouseWheelInPlot(QWheelEvent *event);
    void onMousePressInPlot(QMouseEvent *event);
    void onMouseDoubleClickInPlot(QMouseEvent* event);
    void plotContextMenuRequest(QPoint pos);
    void selectionChangedByUserInPlot();
//    void legendSelectionChanged(QCPLegend::SelectableParts l);
    void saveSelectedGraph();
    void saveAllGraphs();
    void doMeasure();
    void doShift();
    void cleanTracer();
    void cancelMeasure();
    void on_spinPoints_valueChanged(int arg1);                                            // Spin box controls how many data points are collected and displayed
    void on_actionHow_to_use_triggered();
    void on_sendLine_returnPressed();
    void on_clearTermButton_clicked();
    void on_actionShowWidgets_triggered();
    void on_bgColorButton_pressed();
    void on_plotsInfoRadio_clicked(bool checked);

    void on_scrollButton_clicked(bool checked);

    void on_logPlotButton_clicked();
    void plotLabelSelected(bool b);
signals:
    void portOpenFail();                                                                  // Emitted when cannot open port
    void newData(QStringList data);                                                       // Emitted when new data has arrived
    void newPlotData(QStringList data);                                                       // Emitted when new data has arrived

private:
    Ui::MainWindow *ui;
    QColor bgColor = QColor(20,20,20);
    QColor colours[10] = {QColor("#EEEEEE"), QColor("#ffff00"), QColor("#aaffaf"),
                          QColor("#ffaa00"), QColor("#ffaaff"), QColor("#00ffff"),
                          QColor("#ff0000"), QColor("#00aaff"), QColor("#00ff00"),
                          QColor("#ff00aa")};

    enum WheelAction { wheelZoom   = 0x01,
                       wheelShift   = 0x02
                     };
    WheelAction wheelState = wheelZoom;
    enum mouseAction { mouseMove   = 0x01,
                       mouseShift   = 0x02
                     };
    mouseAction mouseState = mouseMove;
    QSharedPointer<QCPAxisTickerText> textTicker;
    QMap<int, yaspGraph*> graphs;
    bool connected;                                                                       // Status connection variable
    bool plotting;                                                                        // Status plotting variable
    bool mousePressed = false;
    int dataPointNumber;                                                                  // Keep track of data points
    QTimer updateTimer;
    QTimer ticksXTimer;
    QTime ticksXTime;
    // Timer used for replotting the plot
    int numberOfAxes;                                                                     // Number of axes for the plot
    double timeBetweenSamples;                                                            // Store time between samples
    QSerialPort *serialPort;                                                              // Serial port; runs in this thread
    QString receivedData;                                                                 // Used for reading from the port
    QString noMsgReceivedData;                                                                 // Used for reading from the port
    QByteArray data;
    int STATE;                                                                            // State of recieiving message from port
    int numberOfPoints;                                                                 // Number of points plotted
    HelpWindow *helpWindow = nullptr;
    DialogWidgets *widgets = nullptr;
    QTabWidget* plotsToolBox = nullptr;
    QVector<graphContainer*> plotsVector;
    QLabel* bottomWidget;
    QFile* logFile = nullptr;
    QTextStream streamLog;
    QFile* logData = nullptr;
    QTextStream streamData;
    QRubberBand* rubberBand = nullptr;
    QPointF rubberOrigin;
    QCPItemTracer *tracer = nullptr;
    bool measureInProgress = false;
    int measureMult;
    double traceerStartKey;
    graphContainer* selectedPlotContainer = nullptr;
    void createUI();                                                                      // Populate the controls
    void enableControls(bool enable);                                                     // Enable/disable controls
    void setupPlot();
    void addPlots();
    void updateLabel(int id, QString info, QColor color);
    bool isColor(QString str);
    // Setup the QCustomPlot
    void cleanGraphs();                                                                                          // Open the inside serial port with these parameters
    void updateGraphNops(bool resetDelta = false);
    void updateGraphParams(QColor plotBgColor);
    int getIdOfQCPGraph(QCPGraph* g);
    void openPort();
    void portOpenedSuccess();                                                             // Called when port opens OK
    void closePort();                                                                  // Called when closing the port
    void setAutoYRange(double r, bool resetDelta = false);
    void addMessageText(QString data, QString color = "black");
    bool checkEndMsgMissed(char cc);
    bool isNumericChar(char cc);
    void updateTracer(int pX);
    void saveDataPlot(QCPGraph* g);
    void shiftPlot(double posY);
    bool startShiftPlot = false;
    double lastPosY = 0;

};


#endif                                                                                    // MAINWINDOW_HPP
