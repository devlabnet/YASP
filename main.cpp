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
**           Author: Cricri042                                             **
**           Contact: Cricri042@devlabnet.eu                               **
**           Date: 08.03.2019                                              **
****************************************************************************/

#include "mainwindow.hpp"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QFile file(":/StyleSheet/StyleSheet/style.qss");                            // Apply style sheet
//    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        a.setStyleSheet(file.readAll());
//        file.close();
//    }

//    QFile f(":qdarkstyle/style.qss");
//    if (!f.exists()) {
//        qDebug() << "Unable to set stylesheet, file not found.";
//    } else {
//        f.open(QFile::ReadOnly | QFile::Text);
//        QTextStream ts(&f);
//        a.setStyleSheet(ts.readAll());
//    }

    MainWindow w;
    QIcon appIcon(":/Icons/Icons/Oscilloscope-128.png");                       // Get the icon for the right corner
    w.setWindowIcon(appIcon);
    w.setWindowTitle("YASP");
    w.show();

    return a.exec();
}
