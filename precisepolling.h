#ifndef PRECISEPOLLING_H
#define PRECISEPOLLING_H

#include <QObject>
#include <QThread>

class PrecisePolling : public QThread {
    Q_OBJECT
public:
    explicit PrecisePolling();
    void StartWork();
    void StopWork();

private:
    qint64 previousPollingTime;
//    QTimer timer;
    bool started = false;
    int period = 500;
    int tCnt = 0;
    int resolution = 1024;
    int waveOut;

protected:
     void run();

signals:
    void send(QString str);
public slots:
//    bool doPolling();

};

#endif // PRECISEPOLLING_H
