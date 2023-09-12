#ifndef POWEROP_H
#define POWEROP_H

#include <QObject>

class PowerOp : public QObject
{
    Q_OBJECT
public:
    explicit PowerOp(QObject *parent = nullptr);

    void shutdown();
    void reboot();
    void sleep();
    void hibernate();
    void closeMonitor();

signals:

};

#endif // POWEROP_H
