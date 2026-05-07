#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include <QCloseEvent>
#include <QMessageBox>
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
    applyPlatformCapabilities();

    ui->widget_tips->setVisible(false);

    TrayIcon::Instance()->setIcon(":/logo.ico");
    TrayIcon::Instance()->setToolTip(this->windowTitle());
    TrayIcon::Instance()->setMainWidget(this);
    TrayIcon::Instance()->setVisible(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_worker->stop();
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
        const QDateTime time = targetDateTime();
        if (shouldUseSystemSchedule()) {
            const PowerResult result = m_powerOp->schedulePowerAction(currentPowerAction(), delaySecondsToTarget(time));
            if (!result.success) {
                ui->pushButton_execute->setChecked(false);
                showNormal();
                QMessageBox::warning(this, tr("Power operation authorization failed"), result.message);
                return;
            }
            m_systemScheduleActive = true;
            m_worker->setTargetDateTime(time);
            m_worker->start();
        } else {
            if (!ensureAuthorizationForCurrentTask()) {
                ui->pushButton_execute->setChecked(false);
                return;
            }
            m_worker->setTargetDateTime(time);
            m_worker->start();
        }

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
        ui->label_bottom_time->setText(time.toString("yyyy-MM-dd HH:mm").toLocal8Bit().data());
    }
    else
    {
        if (m_systemScheduleActive) {
            const PowerResult result = m_powerOp->cancelScheduledPowerActions();
            if (!result.success) {
                ui->pushButton_execute->setChecked(true);
                QMessageBox::warning(this, tr("Power operation cancellation failed"), result.message);
                return;
            }
            m_systemScheduleActive = false;
        }

        ui->pushButton_execute->setText(QObject::tr("Start"));
        ui->widget_left->setEnabled(true);
        ui->widget_right->setEnabled(true);
        ui->widget_tips->setVisible(false);
        if (m_worker->isRunning()) {
            m_worker->stop();
            m_worker->wait();
        }
    }

}

void MainWindow::slotTimeOut()
{
    if (m_systemScheduleActive) {
        m_systemScheduleActive = false;
        ui->pushButton_execute->setChecked(false);
        on_pushButton_execute_clicked(false);
        return;
    }

    PowerResult result{true, QString()};

    switch (eTaskType) {
    case SHUTDOWN:
    {
        result = m_powerOp->shutdown();
        break;
    }
    case REBOOT:
    {
        result = m_powerOp->reboot();
        break;
    }
    case HIBERNATE:
    {
        result = m_powerOp->hibernate();
        break;
    }
    case SLEEP:
    {
        result = m_powerOp->sleep();
        break;
    }
    case CLOSEMONITOR:
    {
        result = m_powerOp->closeMonitor();
        break;
    }
    default:
        break;
    }

    if (!result.success) {
        showNormal();
        QMessageBox::warning(this, tr("Power operation failed"), result.message);
    }

    ui->pushButton_execute->setChecked(false);
    on_pushButton_execute_clicked(false);
}

void MainWindow::applyPlatformCapabilities()
{
    ui->radioButton_sleep->setVisible(m_powerOp->supportsSleep());
    ui->radioButton_hibernate->setVisible(m_powerOp->supportsHibernate());

    if (!ui->radioButton_sleep->isVisible() && eTaskType == SLEEP) {
        eTaskType = SHUTDOWN;
        ui->radioButton_shutdown->setChecked(true);
        ui->label_bottom_task->setText(ui->radioButton_shutdown->text());
    }

    if (!ui->radioButton_hibernate->isVisible() && eTaskType == HIBERNATE) {
        eTaskType = SHUTDOWN;
        ui->radioButton_shutdown->setChecked(true);
        ui->label_bottom_task->setText(ui->radioButton_shutdown->text());
    }
}

PowerAction MainWindow::currentPowerAction() const
{
    switch (eTaskType) {
    case SHUTDOWN:
        return PowerAction::Shutdown;
    case REBOOT:
        return PowerAction::Reboot;
    case HIBERNATE:
        return PowerAction::Hibernate;
    case SLEEP:
        return PowerAction::Sleep;
    case CLOSEMONITOR:
        return PowerAction::CloseMonitor;
    }

    return PowerAction::Shutdown;
}

QDateTime MainWindow::targetDateTime() const
{
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

    return time;
}

int MainWindow::delaySecondsToTarget(const QDateTime &time) const
{
    return qMax(1, static_cast<int>(QDateTime::currentDateTime().secsTo(time)));
}

bool MainWindow::ensureAuthorizationForCurrentTask()
{
    const PowerResult result = m_powerOp->requestAuthorization(currentPowerAction());
    if (!result.success) {
        showNormal();
        QMessageBox::warning(this, tr("Power operation authorization failed"), result.message);
        return false;
    }

    return true;
}

bool MainWindow::shouldUseSystemSchedule() const
{
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    return currentPowerAction() != PowerAction::CloseMonitor;
#else
    return false;
#endif
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
