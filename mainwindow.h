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
    APPOINTEDTIME, //指定时间..
    TIMESPAN       //时间跨度..
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void slotShowWindow();

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
    void on_spinBox_hour_valueChanged(int arg1);
    void on_spinBox_min_valueChanged(int arg1);
    void on_timeEdit_timeChanged(const QTime &time);

private:
    double getWindowScaleFactor();
    void reSetupUi();
    void initDateTime();

private:
    Ui::MainWindow *ui;
    double m_scaleFactor;
    bool m_bHinted = false;
    Worker* m_worker;
    PowerOp* m_powerOp;

    TaskType eTaskType;
    TimeSerial eTimeSerial;
};
#endif // MAINWINDOW_H
