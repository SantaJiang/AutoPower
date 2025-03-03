#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include <QCloseEvent>
#include "trayicon.h"
#include "windows.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //高分辨率屏适配..
    reSetupUi();

    eTaskType = SHUTDOWN;
    eTimeSerial = APPOINTEDTIME;

    initDateTime();

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

double MainWindow::getWindowScaleFactor()
{
    double scaleFactor = 1.0;
    int nWidth = GetSystemMetrics(SM_CXSCREEN);

    if(nWidth >= 3840 )
    {
        scaleFactor = 2.0;
    }
    else if(nWidth < 3840 && nWidth > 1366 )
    {
        scaleFactor = 1.0;
    }
    else if(nWidth <= 1366  && nWidth > 800 )
    {
        scaleFactor = 0.8;
    }
    else if(nWidth <= 800)
    {
        scaleFactor = 0.5;
    }

    return scaleFactor;
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

void MainWindow::slotShowWindow()
{
    if(!m_worker->isRunning())
    {
        m_bHinted = false;
        initDateTime();
    }

    showNormal();
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

void MainWindow::reSetupUi()
{
    double dScaleFactor = getWindowScaleFactor();

    this->setMinimumSize(QSize(555*dScaleFactor, 312*dScaleFactor));
    this->setMaximumSize(QSize(555*dScaleFactor, 312*dScaleFactor));

    ui->horizontalLayout_top->setSpacing(6*dScaleFactor);
    ui->widget_left->setMinimumSize(QSize(0, 20*dScaleFactor));
    ui->verticalLayout_3->setContentsMargins(20*dScaleFactor, 20, 0, 0);

    QFont font;
    font.setFamily(QString::fromUtf8("Microsoft YaHei"));
    font.setPixelSize(24*dScaleFactor);
    font.setBold(false);
    ui->label_taskTypes->setFont(font);
    ui->label_timeSerial->setFont(font);

    QFont font1;
    font1.setFamily(QString::fromUtf8("Microsoft YaHei"));
    font1.setPixelSize(12*dScaleFactor);
    font1.setBold(false);
    ui->radioButton_shutdown->setFont(font1);
    ui->radioButton_closeMonitor->setFont(font1);
    ui->radioButton_fromNow->setFont(font1);
    ui->radioButton_hibernate->setFont(font1);
    ui->radioButton_reboot->setFont(font1);
    ui->radioButton_setTime->setFont(font1);
    ui->radioButton_sleep->setFont(font1);
    ui->pushButton_execute->setFont(font1);
    ui->label_bottom_tip1->setFont(font1);
    ui->label_bottom_task->setFont(font1);
    ui->label_bottom_tip_at->setFont(font1);
    ui->label_bottom_time->setFont(font1);
    ui->dateEdit->setFont(font1);
    ui->timeEdit->setFont(font1);
    ui->spinBox_hour->setFont(font1);
    ui->label_hour->setFont(font1);
    ui->spinBox_min->setFont(font1);
    ui->label_min->setFont(font1);


    ui->verticalSpacer_left_top = new QSpacerItem(20*dScaleFactor, 12*dScaleFactor, QSizePolicy::Minimum, QSizePolicy::Fixed);
    ui->verticalLayout_task->setSpacing(12*dScaleFactor);
    ui->verticalSpacer_left_bottom = new QSpacerItem(20*dScaleFactor, 40*dScaleFactor, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->widget_right->setMinimumSize(QSize(0, 20*dScaleFactor));
    ui->verticalLayout_4->setContentsMargins(20*dScaleFactor, 20*dScaleFactor, 20*dScaleFactor, 0);
    ui->verticalSpacer_right_top = new QSpacerItem(20*dScaleFactor, 12*dScaleFactor, QSizePolicy::Minimum, QSizePolicy::Fixed);
    ui->horizontalLayout_dateTime->setSpacing(6*dScaleFactor);
    ui->horizontalLayout_dateTime->setContentsMargins(-1, -1, 100*dScaleFactor, -1);
    ui->verticalSpacer_right_mindle = new QSpacerItem(20*dScaleFactor, 18*dScaleFactor, QSizePolicy::Minimum, QSizePolicy::Fixed);
    ui->horizontalLayout_fromNowOn->setSpacing(4*dScaleFactor);
    ui->horizontalLayout_fromNowOn->setContentsMargins(-1, 0, 150*dScaleFactor, -1);
    ui->spinBox_hour->setMinimumSize(QSize(60*dScaleFactor, 0));
    ui->horizontalSpacer_space = new QSpacerItem(8*dScaleFactor, 20*dScaleFactor, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->spinBox_min->setMinimumSize(QSize(60*dScaleFactor, 0));
    ui->verticalSpacer_right_bottom = new QSpacerItem(20*dScaleFactor, 40*dScaleFactor, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->horizontalLayout_2->setSpacing(6*dScaleFactor);
    ui->horizontalLayout_2->setContentsMargins(20*dScaleFactor, -1, 20*dScaleFactor, -1);
    ui->label_bottom_time->setMinimumSize(QSize(120*dScaleFactor, 0));
    ui->label_bottom_time->setMaximumSize(QSize(120*dScaleFactor, 16777215));
    ui->horizontalSpacer = new QSpacerItem(40*dScaleFactor, 20*dScaleFactor, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->pushButton_execute->setMinimumSize(QSize(100*dScaleFactor, 30*dScaleFactor));
    ui->pushButton_execute->setMaximumSize(QSize(100*dScaleFactor, 30*dScaleFactor));
}

void MainWindow::initDateTime()
{
    ui->dateEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateEdit->setMinimumDate(QDate::currentDate());
    QTime time = QTime::currentTime();
    time = time.addSecs(600);
    ui->timeEdit->setTime(time);
}

void MainWindow::on_dateEdit_dateChanged(const QDate &date)
{
    ui->radioButton_setTime->setChecked(true);
}

void MainWindow::on_timeEdit_timeChanged(const QTime &time)
{
    ui->radioButton_setTime->setChecked(true);
}

void MainWindow::on_spinBox_hour_valueChanged(int arg1)
{
    ui->radioButton_fromNow->setChecked(true);
}

void MainWindow::on_spinBox_min_valueChanged(int arg1)
{
    ui->radioButton_fromNow->setChecked(true);
}
