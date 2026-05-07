#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QDateTime>
#include <atomic>

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void setTargetDateTime(QDateTime time);
    void stop();
    void run();

signals:
    void timeisup();

private:
    QDateTime m_time;
    std::atomic_bool m_stopRequested{false};
};

#endif // WORKER_H
