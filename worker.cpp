#include "worker.h"
#include <QDebug>

Worker::Worker(QObject *parent)
    : QThread{parent}
{

}

void Worker::setTargetDateTime(QDateTime time)
{
    m_time = time;
}

void Worker::run()
{
    qint64 days = -1;
    qint64 secs = -1;
    bool isRunning = true;
    while (isRunning) {
        days = m_time.daysTo(QDateTime::currentDateTime());
        if(days >= 0)
        {
            secs = m_time.secsTo(QDateTime::currentDateTime());
            if(secs >= 0)
            {
                //时间到了..
                emit timeisup();
                isRunning = false;
            }
        }
        qDebug()<<"days:"<<days<<"secs:"<<secs;
        QThread::sleep(1);
    }
}
