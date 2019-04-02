#include "precisepolling.h"
#include "QDateTime"
#include <QDebug>
#include <window.h>

#include <chrono>
#include <thread>

PrecisePolling::PrecisePolling() :
    previousPollingTime(QDateTime::currentMSecsSinceEpoch()) {
//    timer.setInterval(5);
//    timer.setTimerType(Qt::PreciseTimer);
//    QObject::connect(&timer, &QTimer::timeout, this, &PrecisePolling::doPolling);

}

void PrecisePolling::run() {
    while (started) {
        const qint64 ms = QDateTime::currentMSecsSinceEpoch();
        QString str = QString::number(ms - previousPollingTime);
        qDebug() << str;
        tCnt++;
        if (tCnt >= period) {
            tCnt = 0;
        }
        waveOut = resolution * ((float)tCnt / (float)period);
        QString msg =   "saw " + QString::number(waveOut);

        emit send(msg);
        previousPollingTime = ms;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void PrecisePolling::StartWork() {
    qDebug() << "Start";
    started = true;
    start();
//    timer.start();
}

void PrecisePolling::StopWork() {
    qDebug() << "Stop";
    started = false;
}
