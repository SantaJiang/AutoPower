#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include <QCloseEvent>
#include "trayicon.h"

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

    m_powerOp = new PowerOp(this);

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
        m_powerOp->shutdown();
        break;
    }
    case REBOOT:
    {
        m_powerOp->reboot();
        break;
    }
    case HIBERNATE:
    {
        m_powerOp->hibernate();
        break;
    }
    case SLEEP:
    {
        m_powerOp->sleep();
        break;
    }
    case CLOSEMONITOR:
    {
        m_powerOp->closeMonitor();
        break;
    }
    default:
        break;
    }

    ui->pushButton_execute->setChecked(false);
    on_pushButton_execute_clicked(false);
}


