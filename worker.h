#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QDateTime>

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void setTargetDateTime(QDateTime time);
    void run();

signals:
    void timeisup();

private:
    QDateTime m_time;
};

#endif // WORKER_H
