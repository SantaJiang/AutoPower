#ifndef POWEROP_H
#define POWEROP_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QStringList>

struct PowerResult
{
    bool success = false;
    QString message;
};

enum class PowerAction
{
    Shutdown,
    Reboot,
    Sleep,
    Hibernate,
    CloseMonitor
};

class PowerOp : public QObject
{
    Q_OBJECT
public:
    explicit PowerOp(QObject *parent = nullptr);

    PowerResult shutdown();
    PowerResult reboot();
    PowerResult sleep();
    PowerResult hibernate();
    PowerResult closeMonitor();

    PowerResult requestAuthorization(PowerAction action);
    PowerResult schedulePowerAction(PowerAction action, int delaySeconds);
    PowerResult cancelScheduledPowerActions();
    bool supportsSleep() const;
    bool supportsHibernate() const;

signals:

private:
    QString m_linuxScheduledUnit;
};

#endif // POWEROP_H
