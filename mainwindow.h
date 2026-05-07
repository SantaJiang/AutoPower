#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include "worker.h"
#include "powerop.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum TaskType
{
    SHUTDOWN,
    REBOOT,
    HIBERNATE,
    SLEEP,
    CLOSEMONITOR
};

enum TimeSerial
{
    APPOINTEDTIME,
    TIMESPAN
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_pushButton_execute_clicked(bool checked);
    void on_radioButton_shutdown_clicked();
    void on_radioButton_sleep_clicked();
    void on_radioButton_closeMonitor_clicked();
    void on_radioButton_setTime_clicked();
    void on_radioButton_fromNow_clicked();
    void on_radioButton_hibernate_clicked();
    void on_radioButton_reboot_clicked();
    void slotTimeOut();
    void on_dateEdit_dateChanged(const QDate &date);
    void on_timeEdit_timeChanged(const QTime &time);
    void on_spinBox_hour_valueChanged(int arg1);
    void on_spinBox_min_valueChanged(int arg1);

private:
    void applyPlatformCapabilities();
    PowerAction currentPowerAction() const;
    QDateTime targetDateTime() const;
    int delaySecondsToTarget(const QDateTime &time) const;
    bool ensureAuthorizationForCurrentTask();
    bool shouldUseSystemSchedule() const;

    Ui::MainWindow *ui;
    bool m_bHinted = false;
    bool m_systemScheduleActive = false;
    Worker* m_worker;
    PowerOp* m_powerOp;

    TaskType eTaskType;
    TimeSerial eTimeSerial;
};
#endif // MAINWINDOW_H
