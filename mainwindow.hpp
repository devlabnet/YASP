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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <QSerialPortInfo>
#include "helpwindow.hpp"
#include "dialogwidgets.h"
#include <QToolBox>
#include <QTextEdit>
#include <QTime>
#include "yaspgraph.h"
#include <QNetworkAccessManager>

#define START_MSG      0x10
#define PLOT_MSG       0X11
#define END_MSG        0X12
#define SPACE_MSG      0X13

#define WAIT_START      1
#define IN_MESSAGE      2
#define IN_PLOT_MSG     3
#define UNDEFINED       4

#define DEF_YAXIS_RANGE 4000
#define YAXIS_MAX_RANGE 1000000
#define PLOT_TIME_DEF 15000.0                           // Default time Displayed in Plot in MilliSeconds
#define PLOT_TIME_MIN_DEF 1.0                          // Minimum Range in MilliSeconds
#define PLOT_TIME_MAX_DEF 1000.0 * 3600                 // Maximum Range in MilliSeconds -> 1 Hour
#define PLOT_TIME_MAX_CLEAN_DEF PLOT_TIME_MAX_DEF * 2   // Max Before Clean Range in MilliSeconds

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
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void replot();
    void onNewDataArrived(QStringList newData);                    // Slot for new data from serial port
    void onNewPlotDataArrived(QStringList newData);                // Slot for new data from serial port
    void readData();                                               // Slot for inside serial port
    void dataTerminalReadyChanged(bool dtr);
    void messageSent(QString str);
    void on_comboPort_currentIndexChanged(const QString &arg1);    // Slot displays message on status bar
    void on_connectButton_clicked();                               // Manages connect/disconnect
    void on_stopPlotButton_clicked();                              // Starts and stops plotting
    void on_saveJPGButton_clicked();                               // Button for saving JPG
    void on_resetPlotButton_clicked();                             // Resets plot to initial zoom and coordinates
    void onMouseMoveInPlot(QMouseEvent *event);                    // Displays coordinates of mouse pointer when clicked in plot in status bar
    void onMouseReleaseInPlot(QMouseEvent *event);
    void onMouseWheelInPlot(QWheelEvent *event);
    void onMousePressInPlot(QMouseEvent *event);
    void plotContextMenuRequest(QPoint pos);
    void saveSelectedGraph();
    void saveAllGraphs();
    void resetTracer();
//    void doMenuPlotShiftAction();
    void doMenuPlotColorAction();
#ifdef YASP_PLOTS_WIDTH
    void doMenuPlotWidthAction(int w);
#endif
//    void doMenuPlotScaleAction();
    void doMenuPlotResetAction();
    void doMenuPlotMeasureBoxAction();
    void doMenuPlotMeasureFreqAction();
    void doMenuPlotMeasureAction();
    void doMenuPlotShowHideAction();
    void on_actionHow_to_use_triggered();
    void on_sendLine_returnPressed();
    void on_clearTermButton_clicked();
    void on_actionShowWidgets_triggered();
    void on_bgColorButton_pressed();
    void on_scrollButton_clicked(bool checked);
    void on_logPlotButton_clicked();
    void plotLabelSelectionChanged(bool b);
    void on_spinDisplayTime_valueChanged(double arg1);
    void infoModeLabelSelectionChanged(bool b);
    void mouseWheelTimerShoot();
    void xAxisRangeChanged(const QCPRange& range);
    void yAxisRangeChanged(const QCPRange& range);
    void checkBoxDynamicMeasuresChanged(int state);
    void on_spinDisplayRange_valueChanged(double arg1);
    void checkForUpdateFinished(QNetworkReply* reply);
    void on_saveTermButton_clicked();

    void on_restartDeviceButton_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_aboutNevVersionButton_clicked();

    void on_actionOnline_Documentation_triggered();

    void on_autoScrollCheckBox_stateChanged(int arg1);

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
    double plotTimeInMilliSeconds;
//    double yaspUnit = 1000000.0;
    double yaspUnit = 1000.0;
    QColor bgColor = QColor(20,20,20);
    QColor colours[10] = {QColor("#EEEEEE"), QColor("#ffff00"), QColor("#aaffaf"),
                          QColor("#ffaa00"), QColor("#ffaaff"), QColor("#00ffff"),
                          QColor("#ff0000"), QColor("#00aaff"), QColor("#00ff00"),
                          QColor("#ff00aa")};
    QPoint mousePos;
    enum measureType {  None     = 0x00,
                        Arrow    = 0x01,
                        Box      = 0x02,
                        Freq     = 0x03
                     };
    void resetMouseWheelState();
    QTimer mouseWheelTimer;
    QCPItemText* infoModeLabel = nullptr;
    Qt::MouseButton mouseButtonState;
    QSharedPointer<QCPAxisTickerFixed> fixedTicker;
    QMap<int, yaspGraph*> graphs;
    double lastDataTtime;
    QString downloadUrl = "";
//    double cleanDataTtime;
    yaspGraph* getGraph(int id);
    int selectedPlotId = -1;
    QTimer updateTimer;
    QSerialPort *serialPort;        // Serial port; runs in this thread
    QString receivedData;           // Used for reading from the port
    QString noMsgReceivedData;      // Used for reading from the port
    QByteArray data;
    HelpWindow *helpWindow = nullptr;
    DialogWidgets *widgets = nullptr;
    QFile* logFile = nullptr;
    QTextStream streamLog;
    QFile* logData = nullptr;
    QTextStream streamData;
    QMenu* contextMenu = nullptr;
    QAction* plotShowHideAction;
    QAction* plotMeasureBoxAction;
    QAction* plotMeasureAction;
    // Tracer
    QCPGraph* points;
    QPointF intersectPoint;
    void insertIntersectionPoint(QLineF ref, double x0, double y0, double x1, double y1);
    measureType measureMode = measureType::None;
    double measureMult;
    double tracerStartKey;
    double lastTracerXValueRef;
    double lastTracerXValueTracer;
    QCPItemTracer *tracer = nullptr;
    QCPItemRect* tracerRect;
    QCPItemStraightLine* traceLineBottom;
    QCPItemStraightLine* traceLineTop;
    QCPItemStraightLine* traceLineLeft;
    QCPItemStraightLine* traceLineRight;
//    QCPItemLine* refLine;
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
    QList<QCPItemText*> tracerHLinesRefInfo;
    QList<QCPItemText*> tracerHLinesTracerInfo;
    int tracerStep = 0;
    void createUI();
    void enableControls(bool enable);
    void loadHelpFile();
    void initTracer();
    yaspGraph *addGraph(int id);
    void updateLabel(int id, QString plotInfoStr);
    void doContextMenuHeader(yaspGraph* yGraph);
    bool isColor(QString str);
    void cleanGraphs();
    void openPort();
    void portOpenedSuccess();
    void closePort();
    void addMessageText(QString data, QString color = "black");
    bool checkEndMsgMissed(char cc);
    bool isNumericChar(char cc);
    void updateTracerBox();
    void updateTracerMeasure();
    void updateTracerFrequency();
    void saveDataPlot();
    void shiftPlot(int pY);
    void scalePlot(double scale);
    void unselectGraphs();
    void cleanDataGraphsBefore(double d);
    void cleanDataGraphs();
    void ShowPlotsExceptWG(bool show);
    void togglePlotsVisibility(bool show);
    bool startShiftPlot = false;
    double lastPosY = 0;
    int lastY = 0;
    double scaleMult = 1.0;
    yaspGraph* workingGraph = nullptr;
    bool compareDouble(double value1, double value2, quint8 precision) {
        return std::abs(value1 - value2) < std::pow(10, -precision);
    }
    // Check Update Version
    void checkForUpdate();
    bool compareVersions(const QString& x, const QString& y);
    void setUpdateAvailable(bool available, QString latestVersion="", QString changelog="");
//    bool boolTest;
//    double round(long double number, int precision) {
//      int decimals = std::pow(10, precision);
//      return (std::round(number * decimals)) / decimals;
//    }

};


#endif // MAINWINDOW_HPP
