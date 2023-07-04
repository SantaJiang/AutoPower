#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include <QCloseEvent>
#include <Windows.h>
#include "powrprof.h"
#include "winuser.h"
#include "processthreadsapi.h"
#include "trayicon.h"
#pragma comment(lib,"PowrProf.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Advapi32.lib")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    eTaskType = SHUTDOWN;
    eTimeSerial = APPOINTEDTIME;

    ui->dateEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateEdit->setMinimumDate(QDate::currentDate());
    QTime time = QTime::currentTime();
    time = time.addSecs(600);
    ui->timeEdit->setTime(time);

    m_worker = new Worker(this);
    connect(m_worker, SIGNAL(timeisup()), this, SLOT(slotTimeOut()));

    ui->widget_tips->setVisible(false);

    //set trayicon
    TrayIcon::Instance()->setIcon(":/logo.ico");
    TrayIcon::Instance()->setToolTip(this->windowTitle());
    TrayIcon::Instance()->setMainWidget(this);
    TrayIcon::Instance()->setVisible(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_worker->terminate();
    m_worker->wait();
}

void MainWindow::on_radioButton_shutdown_clicked()
{
    eTaskType = SHUTDOWN;
    ui->label_bottom_task->setText(ui->radioButton_shutdown->text());
}

void MainWindow::on_radioButton_reboot_clicked()
{
    eTaskType = REBOOT;
    ui->label_bottom_task->setText(ui->radioButton_reboot->text());
}

void MainWindow::on_radioButton_hibernate_clicked()
{
    eTaskType = HIBERNATE;
    ui->label_bottom_task->setText(ui->radioButton_hibernate->text());
}

void MainWindow::on_radioButton_sleep_clicked()
{
    eTaskType = SLEEP;
    ui->label_bottom_task->setText(ui->radioButton_sleep->text());
}

void MainWindow::on_radioButton_closeMonitor_clicked()
{
    eTaskType = CLOSEMONITOR;
    ui->label_bottom_task->setText(ui->radioButton_closeMonitor->text());
}

void MainWindow::on_radioButton_setTime_clicked()
{
    eTimeSerial = APPOINTEDTIME;
}

void MainWindow::on_radioButton_fromNow_clicked()
{
    eTimeSerial = TIMESPAN;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(ui->pushButton_execute->isChecked())
    {
        event->ignore();
        this->hide();
        return;
    }
    return QMainWindow::closeEvent(event);
}

void MainWindow::on_pushButton_execute_clicked(bool checked)
{
    if(checked)
    {
        ui->pushButton_execute->setText(QObject::tr("Running.."));
        ui->widget_left->setEnabled(false);
        ui->widget_right->setEnabled(false);
        ui->widget_tips->setVisible(true);
        if(!m_bHinted)
        {
            TrayIcon::Instance()->showMessage(this->windowTitle(), tr("minimized to tray"));
            m_bHinted = true;
        }
        this->hide();

        QDateTime time;
        switch (eTimeSerial) {
        case APPOINTEDTIME:
        {
            time.setDate(ui->dateEdit->date());
            time.setTime(ui->timeEdit->time());
            break;
        }
        case TIMESPAN:
        {
            time = QDateTime::currentDateTime();
            qint64 cecs = ui->spinBox_hour->value()*60*60 + ui->spinBox_min->value()*60;
            time = time.addSecs(cecs);
            break;
        }
        default:
            break;
        }

        ui->label_bottom_time->setText(time.toString("yyyy-MM-dd HH:mm").toLocal8Bit().data());
        m_worker->setTargetDateTime(time);
        m_worker->start();
    }
    else
    {
        ui->pushButton_execute->setText(QObject::tr("Start"));
        ui->widget_left->setEnabled(true);
        ui->widget_right->setEnabled(true);
        ui->widget_tips->setVisible(false);
        m_worker->terminate();
        m_worker->wait();
    }

}

void MainWindow::slotTimeOut()
{
    switch (eTaskType) {
    case SHUTDOWN:
    {
        shutdown();
        break;
    }
    case REBOOT:
    {
        reboot();
        break;
    }
    case HIBERNATE:
    {
        hibernate();
        break;
    }
    case SLEEP:
    {
        sleep();
        break;
    }
    case CLOSEMONITOR:
    {
        closeMonitor();
        break;
    }
    default:
        break;
    }
}

void MainWindow::shutdown()
{
    //强行关机..
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    //获取进程标志..
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;
    //获取关机特权的LUID..
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,    &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    //获取这个进程的关机特权..
    AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS)
        return;
    // 强制关闭计算机..
    if ( !ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0))
        return;

    return;
}

void MainWindow::reboot()
{
    //重启计算机..
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    //获取进程标志..
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;
    //获取关机特权的LUID..
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,    &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    //获取这个进程的关机特权..
    AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS)
        return;
    // 强制重启计算机..
    if ( !ExitWindowsEx(EWX_REBOOT| EWX_FORCE, 0))
        return;

    return;
}

void MainWindow::hibernate()
{
    // 让系统进入休眠 ..
    SetSuspendState(TRUE, TRUE, FALSE);
}

void MainWindow::sleep()
{
    // 让系统进入睡眠..
    SetSuspendState(FALSE, TRUE, FALSE);
}

void MainWindow::closeMonitor()
{
    //关闭显示器..
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) 2);
}
