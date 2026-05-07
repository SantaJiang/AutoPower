#include "worker.h"
#include <QDebug>

Worker::Worker(QObject *parent)
    : QThread{parent}
{

}

void Worker::setTargetDateTime(QDateTime time)
{
    m_time = time;
    m_stopRequested = false;
}

void Worker::stop()
{
    m_stopRequested = true;
}

void Worker::run()
{
    while (!m_stopRequested) {
        const qint64 secs = m_time.secsTo(QDateTime::currentDateTime());
        qDebug() << "secs:" << secs;
        if (secs >= 0) {
            emit timeisup();
            return;
        }
        QThread::sleep(1);
    }
}
