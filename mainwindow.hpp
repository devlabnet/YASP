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

#define START_MSG       '$'
#define PLOT_MSG       '@'
#define END_MSG         ';'

#define WAIT_START      1
#define IN_MESSAGE      2
#define IN_PLOT_MSG     3
#define UNDEFINED       4

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
    void replot();                                                                        // Slot for repainting the plot
    void onNewDataArrived(QStringList newData);                                           // Slot for new data from serial port
    void onNewPlotDataArrived(QStringList newData);                                       // Slot for new data from serial port
    void readData();                                                                      // Slot for inside serial port
    void plotColorChanged(int tabInd, QColor color);

    void on_comboPort_currentIndexChanged(const QString &arg1);                           // Slot displays message on status bar
    void on_connectButton_clicked();                                                      // Manages connect/disconnect
    void on_stopPlotButton_clicked();                                                     // Starts and stops plotting
    void on_comboAxes_currentIndexChanged(int index);                                     // Display number of axes and colors in status bar
    void on_saveJPGButton_clicked();                                                      // Button for saving JPG
    void on_resetPlotButton_clicked();                                                    // Resets plot to initial zoom and coordinates
    void onMouseMoveInPlot(QMouseEvent *event);                                           // Displays coordinates of mouse pointer when clicked in plot in status bar
    void onMouseReleaseInPlot(QMouseEvent *event);
    void onMouseWheelInPlot(QWheelEvent *event);
    void on_spinPoints_valueChanged(int arg1);                                            // Spin box controls how many data points are collected and displayed
    void on_actionHow_to_use_triggered();
    void on_sendLine_returnPressed();
    void on_clearTermButton_clicked();
    void on_actionShowWidgets_triggered();
    void on_bgColorButton_pressed();
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
    bool connected;                                                                       // Status connection variable
    bool plotting;                                                                        // Status plotting variable
    int dataPointNumber;                                                                  // Keep track of data points
    QTimer updateTimer;                                                                   // Timer used for replotting the plot
    int numberOfAxes;                                                                     // Number of axes for the plot
    QTime timeOfFirstData;                                                                // Record the time of the first data point
    double timeBetweenSamples;                                                            // Store time between samples
    QSerialPort *serialPort;                                                              // Serial port; runs in this thread
    QString receivedData;                                                                 // Used for reading from the port
    QString noMsgReceivedData;                                                                 // Used for reading from the port
    QByteArray data;
    int STATE;                                                                            // State of recieiving message from port
    int NUMBER_OF_POINTS;                                                                 // Number of points plotted
    HelpWindow *helpWindow = nullptr;
    DialogWidgets *widgets = nullptr;
    QTabWidget* plotsToolBox = nullptr;
    void createUI();                                                                      // Populate the controls
    void enableControls(bool enable);                                                     // Enable/disable controls
    void setupPlot();
    void addPlots();
    bool isColor(QString str);
    // Setup the QCustomPlot
    void cleanGraphs();                                                                                          // Open the inside serial port with these parameters
    void updateGraphs();
    void openPort();
    void portOpenedSuccess();                                                             // Called when port opens OK
    void closePort();                                                                  // Called when closing the port
    void setAutoYRange(double r);
    void addMessageText(QString data, QString color = "black");
    bool checkEndMsgMissed(unsigned char cc);
};


#endif                                                                                    // MAINWINDOW_HPP
