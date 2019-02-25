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
#include <QTime>
//#include <QRubberBand>
#include "yaspgraph.h"

#define START_MSG      0x10
#define PLOT_MSG       0X11
#define END_MSG        0X12
#define SPACE_MSG      0X13

#define WAIT_START      1
#define IN_MESSAGE      2
#define IN_PLOT_MSG     3
#define UNDEFINED       4

#define DEF_YAXIS_RANGE 1500
#define PLOT_TIME_DEF 15            // Default time Displayed in Plot in Seconds
#define PLOT_TIME_MIN_DEF 10.0      // Seconds
#define PLOT_TIME_MAX_DEF 600.0    // Seconds
#define PLOT_TIME_STEP_DEF 10.0     // Seconds

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void replot();
//    void addTickX();
    // Slot for repainting the plot
    void onNewDataArrived(QStringList newData);                                           // Slot for new data from serial port
    void onNewPlotDataArrived(QStringList newData);                                       // Slot for new data from serial port
    void readData();                                                                      // Slot for inside serial port
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
//    void onMouseDoubleClickInPlot(QMouseEvent* event);
    void plotContextMenuRequest(QPoint pos);
//    void selectionChangedByUserInPlot();
//    void legendSelectionChanged(QCPLegend::SelectableParts l);
    void saveSelectedGraph();
    void saveAllGraphs();
//    void doMeasure();
//    void doShift();
    void cleanTracer();
    void doMenuCloseAction(bool);
    void doMenuPlotShiftAction();
    void doMenuPlotColorAction();
    void doMenuPlotScaleAction();
    void doMenuPlotResetAction();
    void doMenuPlotMeasureAction();
    void doMenuPlotShowHideAction();
    void on_actionHow_to_use_triggered();
    void on_sendLine_returnPressed();
    void on_clearTermButton_clicked();
    void on_actionShowWidgets_triggered();
    void on_bgColorButton_pressed();
    void contextMenuTriggered(QAction*);
    void menuAboutToHide();
    void on_scrollButton_clicked(bool checked);
    void on_logPlotButton_clicked();
    void plotLabelSelected(bool b);
    void xAxisRangeChanged(const QCPRange& range);
    void on_spinDisplayTime_valueChanged(double arg1);

signals:
    void portOpenFail();                                                                  // Emitted when cannot open port
    void newData(QStringList data);                                                       // Emitted when new data has arrived
    void newPlotData(QStringList data);                                                       // Emitted when new data has arrived

private:
    Ui::MainWindow *ui;
    bool connected;                                                                       // Status connection variable
    bool plotting;                                                                        // Status plotting variable
    int dataPointNumber;                                                                  // Keep track of data points
    int numberOfAxes;                                                                     // Number of axes for the plot
    int STATE;                                                                            // State of recieiving message from port
    double plotTimeInSeconds;

    QColor bgColor = QColor(20,20,20);
    QColor colours[10] = {QColor("#EEEEEE"), QColor("#ffff00"), QColor("#aaffaf"),
                          QColor("#ffaa00"), QColor("#ffaaff"), QColor("#00ffff"),
                          QColor("#ff0000"), QColor("#00aaff"), QColor("#00ff00"),
                          QColor("#ff00aa")};

    enum WheelAction { wheelNone        = 0x00,
                       wheelZoom        = 0x01,
                       wheelScalePlot   = 0x02
                     };
    WheelAction wheelState = wheelZoom;
    enum mouseAction { mouseNone   = 0x00,
                       mouseMove   = 0x01,
                       mouseShift   = 0x02,
                       mouseDoMesure = 0x03
                     };
    mouseAction mouseState = mouseMove;
    void resetMouseWheelState();
    QCPItemText* infoModeLabel = nullptr;
    QCP::Interactions plotInteractions;
//    bool mousePressed = false;
    Qt::MouseButton mouseButtonState;
//    QSharedPointer<QCPAxisTickerText> textTicker;
    QSharedPointer<QCPAxisTickerFixed> fixedTicker;
    QMap<int, yaspGraph*> graphs;
//    QMap<int, int> pointTime;
    double lastDataTtime;
    double cleanDataTtime;
//    QString plotInfoStr;
    yaspGraph* getGraph(int id);
//    QVector<qreal> plotDashPattern;
//    QVector<qreal> rLineDashPattern;
    int selectedPlotId = -1;
    QTimer updateTimer;
//    QTimer ticksXTimer;
//    QTime ticksXTime;
    // Timer used for replotting the plot
    QSerialPort *serialPort;                                                              // Serial port; runs in this thread
    QString receivedData;                                                                 // Used for reading from the port
    QString noMsgReceivedData;                                                                 // Used for reading from the port
    QByteArray data;
    HelpWindow *helpWindow = nullptr;
    DialogWidgets *widgets = nullptr;
    QFile* logFile = nullptr;
    QTextStream streamLog;
    QFile* logData = nullptr;
    QTextStream streamData;
//    QHBoxLayout* groupBoxPlotSelectionHbox;
//    QButtonGroup* selectButtonsGroup;
    QMenu* contextMenu = nullptr;
    QAction* plotShowHideAction;
    QAction* plotMeasureAction;
//    QRubberBand* rubberBand = nullptr;
//    QPointF rubberOrigin;
    // Tracer
    QCPItemTracer *tracer = nullptr;
    QCPItemRect* tracerRect;
    bool measureInProgress = false;
    double measureMult;
    double tracerStartKey;
    double lastTracerXValueRef;
    double lastTracerXValueTracer;
    QCPItemLine* traceLineBottom;
    QCPItemLine* traceLineTop;
    QCPItemLine* refLine;
    QCPItemLine* tracerArrowAmplitude;
    QCPItemText* tracerArrowAmplitudeTxt;
    QCPItemLine* tracerArrowAmplitudeTop;
    QCPItemText* tracerArrowAmplitudeTopTxt;
    QCPItemLine* tracerArrowAmplitudeBottom;
    QCPItemText* tracerArrowAmplitudeBottomTxt;
    QCPItemLine* tracerArrowFromRef;
    QCPItemText* tracerArrowFromRefTxt;
    QList<QCPItemLine*> tracerHLinesRef;
    QList<QCPItemLine*> tracerHLinesTracer;
    void createUI();                                                                      // Populate the controls
    void enableControls(bool enable);                                                     // Enable/disable controls
    yaspGraph *addGraph(int id);
    void updateLabel(int id, QString plotInfoStr);
    void doContextMenuHeader(yaspGraph* yGraph);
    bool isColor(QString str);
    // Setup the QCustomPlot
    void cleanGraphs();                                                                                          // Open the inside serial port with these parameters
    void openPort();
    void portOpenedSuccess();                                                             // Called when port opens OK
    void closePort();                                                                  // Called when closing the port
    void addMessageText(QString data, QString color = "black");
    bool checkEndMsgMissed(char cc);
    bool isNumericChar(char cc);
    void updateTracer(int pX);
    void saveDataPlot(yaspGraph* yGraph);
    void shiftPlot(int pY);
    void scalePlot(double scale);
    void unselectGraphs();
    void cleanGraphsBefore(double d);
    void ShowPlotsExceptWG(bool show);
    bool startShiftPlot = false;
//    bool startScalePlot = false;
    double lastPosY = 0;
    int lastY = 0;
    double scaleMult = 1.0;
    yaspGraph* workingGraph = nullptr;

    bool compareDouble(double value1, double value2, quint8 precision) {
        return std::abs(value1 - value2) < std::pow(10, -precision);
    }

//    double round(long double number, int precision) {
//      int decimals = std::pow(10, precision);
//      return (std::round(number * decimals)) / decimals;
//    }

};


#endif                                                                                    // MAINWINDOW_HPP
