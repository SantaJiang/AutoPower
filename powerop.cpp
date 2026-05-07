#include "powerop.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <Windows.h>
#include "powrprof.h"
#include "winuser.h"
#include "processthreadsapi.h"
#pragma comment(lib,"PowrProf.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Advapi32.lib")
#elif defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#endif

namespace {

PowerResult success()
{
    return {true, QString()};
}

PowerResult failure(const QString &message)
{
    return {false, message};
}

#ifdef Q_OS_WIN
QString windowsErrorMessage(DWORD error)
{
    LPWSTR buffer = nullptr;
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                        | FORMAT_MESSAGE_FROM_SYSTEM
                                        | FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr,
                                        error,
                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                        reinterpret_cast<LPWSTR>(&buffer),
                                        0,
                                        nullptr);
    QString message;
    if (length > 0 && buffer != nullptr) {
        message = QString::fromWCharArray(buffer).trimmed();
        LocalFree(buffer);
    }

    if (message.isEmpty()) {
        message = QStringLiteral("Windows error %1").arg(error);
    }
    return message;
}

PowerResult enableShutdownPrivilege()
{
    HANDLE token = nullptr;
    TOKEN_PRIVILEGES privileges;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return failure(PowerOp::tr("Unable to open the process token: %1").arg(windowsErrorMessage(GetLastError())));
    }

    if (!LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid)) {
        const DWORD error = GetLastError();
        CloseHandle(token);
        return failure(PowerOp::tr("Unable to query shutdown privilege: %1").arg(windowsErrorMessage(error)));
    }

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr, 0)) {
        const DWORD error = GetLastError();
        CloseHandle(token);
        return failure(PowerOp::tr("Unable to enable shutdown privilege: %1").arg(windowsErrorMessage(error)));
    }

    const DWORD privilegeError = GetLastError();
    CloseHandle(token);
    if (privilegeError == ERROR_NOT_ALL_ASSIGNED) {
        return failure(PowerOp::tr("The current user does not have permission to shut down or restart this computer."));
    }
    if (privilegeError != ERROR_SUCCESS) {
        return failure(PowerOp::tr("Unable to enable shutdown privilege: %1").arg(windowsErrorMessage(privilegeError)));
    }

    return success();
}
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
PowerResult runCommand(const QString &program,
                       const QStringList &arguments,
                       const QString &failurePrefix,
                       int timeoutMs = 10000)
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted()) {
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, process.errorString()));
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, PowerOp::tr("operation timed out")));
    }

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        return success();
    }

    QString detail = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    if (detail.isEmpty()) {
        detail = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    }
    if (detail.isEmpty()) {
        detail = process.errorString();
    }

    return failure(QStringLiteral("%1: %2").arg(failurePrefix, detail));
}
#endif

#ifdef Q_OS_MACOS
PowerResult runAppleScript(const QString &script, const QString &failurePrefix, int timeoutMs = 15000)
{
    return runCommand(QStringLiteral("osascript"),
                      QStringList() << QStringLiteral("-e") << script,
                      failurePrefix,
                      timeoutMs);
}

QString appleScriptQuoted(const QString &value)
{
    QString quoted = value;
    quoted.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    quoted.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    return QStringLiteral("\"%1\"").arg(quoted);
}

QString shellQuoted(const QString &value)
{
    QString quoted = value;
    quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QStringLiteral("'%1'").arg(quoted);
}

PowerResult runAdministratorShellCommand(const QString &command, const QString &failurePrefix)
{
    return runAppleScript(QStringLiteral("do shell script %1 with administrator privileges")
                          .arg(appleScriptQuoted(command)),
                          failurePrefix,
                          120000);
}

QString macosActionCommand(PowerAction action)
{
    switch (action) {
    case PowerAction::Shutdown:
        return QStringLiteral("/sbin/shutdown -h now");
    case PowerAction::Reboot:
        return QStringLiteral("/sbin/shutdown -r now");
    case PowerAction::Sleep:
        return QStringLiteral("/usr/bin/pmset sleepnow");
    case PowerAction::Hibernate:
    case PowerAction::CloseMonitor:
        return QString();
    }

    return QString();
}

#endif

#ifdef Q_OS_LINUX
QDBusInterface login1Interface()
{
    return QDBusInterface(QStringLiteral("org.freedesktop.login1"),
                          QStringLiteral("/org/freedesktop/login1"),
                          QStringLiteral("org.freedesktop.login1.Manager"),
                          QDBusConnection::systemBus());
}

PowerResult callLogin1(const QString &method, bool interactiveAuthorization, const QString &failurePrefix)
{
    if (!QDBusConnection::systemBus().isConnected()) {
        return failure(PowerOp::tr("Unable to connect to the system D-Bus bus."));
    }

    QDBusInterface interface = login1Interface();
    if (!interface.isValid()) {
        return failure(PowerOp::tr("systemd-logind is not available: %1").arg(interface.lastError().message()));
    }

    const QDBusMessage reply = interface.call(method, interactiveAuthorization);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        QString detail = reply.errorMessage();
        if (detail.isEmpty()) {
            detail = reply.errorName();
        }
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, detail));
    }

    return success();
}

QString login1Capability(const QString &method)
{
    if (!QDBusConnection::systemBus().isConnected()) {
        return QStringLiteral("unavailable");
    }

    QDBusInterface interface = login1Interface();
    if (!interface.isValid()) {
        return QStringLiteral("unavailable");
    }

    const QDBusReply<QString> reply = interface.call(method);
    if (!reply.isValid()) {
        return QStringLiteral("unavailable");
    }

    return reply.value();
}

bool login1CapabilityAllowed(const QString &method)
{
    const QString value = login1Capability(method);
    return value == QLatin1String("yes") || value == QLatin1String("challenge");
}

PowerResult requestLogin1Authorization(const QString &capabilityMethod,
                                       const QString &polkitAction,
                                       const QString &failurePrefix)
{
    const QString capability = login1Capability(capabilityMethod);
    if (capability == QLatin1String("yes")) {
        return success();
    }
    if (capability != QLatin1String("challenge")) {
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, PowerOp::tr("the current session does not allow this action")));
    }

    const QString pkcheckPath = QStandardPaths::findExecutable(QStringLiteral("pkcheck"));
    if (pkcheckPath.isEmpty()) {
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, PowerOp::tr("pkcheck is not available for interactive authorization")));
    }

    return runCommand(pkcheckPath,
                      QStringList() << QStringLiteral("--process")
                                    << QString::number(QCoreApplication::applicationPid())
                                    << QStringLiteral("--allow-user-interaction")
                                    << QStringLiteral("--action-id")
                                    << polkitAction,
                      failurePrefix,
                      60000);
}

QString linuxUnitName(PowerAction action)
{
    switch (action) {
    case PowerAction::Shutdown:
        return QStringLiteral("autopower-shutdown");
    case PowerAction::Reboot:
        return QStringLiteral("autopower-reboot");
    case PowerAction::Sleep:
        return QStringLiteral("autopower-suspend");
    case PowerAction::Hibernate:
        return QStringLiteral("autopower-hibernate");
    case PowerAction::CloseMonitor:
        return QStringLiteral("autopower-close-monitor");
    }

    return QStringLiteral("autopower");
}

QString linuxSystemctlCommand(PowerAction action)
{
    switch (action) {
    case PowerAction::Shutdown:
        return QStringLiteral("poweroff");
    case PowerAction::Reboot:
        return QStringLiteral("reboot");
    case PowerAction::Sleep:
        return QStringLiteral("suspend");
    case PowerAction::Hibernate:
        return QStringLiteral("hibernate");
    case PowerAction::CloseMonitor:
        return QString();
    }

    return QString();
}

PowerResult runPkexec(const QStringList &arguments, const QString &failurePrefix, int timeoutMs = 60000)
{
    const QString pkexecPath = QStandardPaths::findExecutable(QStringLiteral("pkexec"));
    if (pkexecPath.isEmpty()) {
        return failure(QStringLiteral("%1: %2").arg(failurePrefix, PowerOp::tr("pkexec is not available for interactive authorization")));
    }

    return runCommand(pkexecPath, arguments, failurePrefix, timeoutMs);
}
#endif

}

PowerOp::PowerOp(QObject *parent)
    : QObject{parent}
{

}

PowerResult PowerOp::shutdown()
{

#ifdef Q_OS_WIN
    const PowerResult privilege = enableShutdownPrivilege();
    if (!privilege.success) {
        return privilege;
    }
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCEIFHUNG,
                       SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED)) {
        return failure(tr("Unable to shut down: %1").arg(windowsErrorMessage(GetLastError())));
    }
    return success();
#elif defined(Q_OS_MACOS)
    return runAdministratorShellCommand(QStringLiteral("/sbin/shutdown -h now"),
                                        tr("Unable to shut down"));
#elif defined(Q_OS_LINUX)
    return callLogin1(QStringLiteral("PowerOff"), false, tr("Unable to shut down"));
#else
    return failure(tr("Shutdown is not supported on this platform."));
#endif
}

PowerResult PowerOp::reboot()
{

#ifdef Q_OS_WIN
    const PowerResult privilege = enableShutdownPrivilege();
    if (!privilege.success) {
        return privilege;
    }
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG,
                       SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED)) {
        return failure(tr("Unable to restart: %1").arg(windowsErrorMessage(GetLastError())));
    }
    return success();
#elif defined(Q_OS_MACOS)
    return runAdministratorShellCommand(QStringLiteral("/sbin/shutdown -r now"),
                                        tr("Unable to restart"));
#elif defined(Q_OS_LINUX)
    return callLogin1(QStringLiteral("Reboot"), false, tr("Unable to restart"));
#else
    return failure(tr("Restart is not supported on this platform."));
#endif
}

PowerResult PowerOp::hibernate()
{

#ifdef Q_OS_WIN
    if (!SetSuspendState(TRUE, TRUE, FALSE)) {
        return failure(tr("Unable to hibernate: %1").arg(windowsErrorMessage(GetLastError())));
    }
    return success();
#elif defined(Q_OS_MACOS)
    return failure(tr("Hibernate is not supported on macOS."));
#elif defined(Q_OS_LINUX)
    return callLogin1(QStringLiteral("Hibernate"), false, tr("Unable to hibernate"));
#else
    return failure(tr("Hibernate is not supported on this platform."));
#endif
}

PowerResult PowerOp::sleep()
{

#ifdef Q_OS_WIN
    if (!SetSuspendState(FALSE, TRUE, FALSE)) {
        return failure(tr("Unable to sleep: %1").arg(windowsErrorMessage(GetLastError())));
    }
    return success();
#elif defined(Q_OS_MACOS)
    return runCommand(QStringLiteral("pmset"),
                      QStringList() << QStringLiteral("sleepnow"),
                      tr("Unable to sleep"));
#elif defined(Q_OS_LINUX)
    return callLogin1(QStringLiteral("Suspend"), false, tr("Unable to sleep"));
#else
    return failure(tr("Sleep is not supported on this platform."));
#endif
}

PowerResult PowerOp::closeMonitor()
{

#ifdef Q_OS_WIN
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) 2);
    return success();
#elif defined(Q_OS_MACOS)
    return runCommand(QStringLiteral("pmset"),
                      QStringList() << QStringLiteral("displaysleepnow"),
                      tr("Unable to turn off the display"));
#elif defined(Q_OS_LINUX)
    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString xsetPath = QStandardPaths::findExecutable(QStringLiteral("xset"));
    if (!environment.value(QStringLiteral("DISPLAY")).isEmpty() && !xsetPath.isEmpty()) {
        return runCommand(xsetPath,
                          QStringList() << QStringLiteral("dpms") << QStringLiteral("force") << QStringLiteral("off"),
                          tr("Unable to turn off the display"));
    }
    return failure(tr("Turning off the display is only supported on Linux X11 sessions with xset installed."));
#else
    return failure(tr("Turning off the display is not supported on this platform."));
#endif
}

PowerResult PowerOp::requestAuthorization(PowerAction action)
{
#ifdef Q_OS_WIN
    if (action == PowerAction::Shutdown || action == PowerAction::Reboot) {
        return enableShutdownPrivilege();
    }
    return success();
#elif defined(Q_OS_MACOS)
    Q_UNUSED(action)
    return success();
#elif defined(Q_OS_LINUX)
    switch (action) {
    case PowerAction::Shutdown:
        return requestLogin1Authorization(QStringLiteral("CanPowerOff"),
                                          QStringLiteral("org.freedesktop.login1.power-off"),
                                          tr("Unable to request shutdown authorization"));
    case PowerAction::Reboot:
        return requestLogin1Authorization(QStringLiteral("CanReboot"),
                                          QStringLiteral("org.freedesktop.login1.reboot"),
                                          tr("Unable to request restart authorization"));
    case PowerAction::Sleep:
        return requestLogin1Authorization(QStringLiteral("CanSuspend"),
                                          QStringLiteral("org.freedesktop.login1.suspend"),
                                          tr("Unable to request sleep authorization"));
    case PowerAction::Hibernate:
        return requestLogin1Authorization(QStringLiteral("CanHibernate"),
                                          QStringLiteral("org.freedesktop.login1.hibernate"),
                                          tr("Unable to request hibernate authorization"));
    case PowerAction::CloseMonitor:
        return success();
    }
    return failure(tr("Unsupported power operation."));
#else
    Q_UNUSED(action)
    return failure(tr("Power operations are not supported on this platform."));
#endif
}

PowerResult PowerOp::schedulePowerAction(PowerAction action, int delaySeconds)
{
#ifdef Q_OS_MACOS
    if (action == PowerAction::Hibernate) {
        return failure(tr("Hibernate is not supported on macOS."));
    }
    if (action == PowerAction::CloseMonitor) {
        return failure(tr("Close monitor cannot be scheduled as a privileged macOS system power action."));
    }

    const QString actionCommand = macosActionCommand(action);
    if (actionCommand.isEmpty()) {
        return failure(tr("Unsupported power operation."));
    }

    const QString pidPath = QStringLiteral("/private/tmp/autopower-scheduled-power.pid");
    const QString command = QStringLiteral(
        "if [ -f %1 ]; then /bin/kill \"$(/bin/cat %1)\" >/dev/null 2>&1 || true; /bin/rm -f %1; fi\n"
        "(/bin/sleep %2; /bin/rm -f %1; %3) >/dev/null 2>&1 &\n"
        "/bin/echo $! > %1")
        .arg(shellQuoted(pidPath))
        .arg(qMax(1, delaySeconds))
        .arg(actionCommand);
    return runAdministratorShellCommand(command, tr("Unable to schedule macOS power action"));
#elif defined(Q_OS_LINUX)
    if (action == PowerAction::CloseMonitor) {
        return failure(tr("Close monitor cannot be scheduled as a privileged Linux system power action."));
    }

    if (!login1CapabilityAllowed(action == PowerAction::Shutdown ? QStringLiteral("CanPowerOff")
                                  : action == PowerAction::Reboot ? QStringLiteral("CanReboot")
                                  : action == PowerAction::Sleep ? QStringLiteral("CanSuspend")
                                  : QStringLiteral("CanHibernate"))) {
        return failure(tr("The selected power action is not currently available on this Linux session."));
    }

    const QString systemdRunPath = QStandardPaths::findExecutable(QStringLiteral("systemd-run"));
    const QString systemctlPath = QStandardPaths::findExecutable(QStringLiteral("systemctl"));
    if (systemdRunPath.isEmpty() || systemctlPath.isEmpty()) {
        return failure(tr("systemd-run and systemctl are required to schedule Linux power actions."));
    }

    const QString command = linuxSystemctlCommand(action);
    if (command.isEmpty()) {
        return failure(tr("Unsupported power operation."));
    }

    const QString unitName = linuxUnitName(action);
    QStringList arguments;
    arguments << systemdRunPath
              << QStringLiteral("--system")
              << QStringLiteral("--unit=%1").arg(unitName)
              << QStringLiteral("--description=AutoPower scheduled power action")
              << QStringLiteral("--on-active=%1s").arg(qMax(1, delaySeconds))
              << systemctlPath
              << command;

    const PowerResult result = runPkexec(arguments, tr("Unable to schedule Linux power action"));
    if (result.success) {
        m_linuxScheduledUnit = unitName;
    }
    return result;
#else
    Q_UNUSED(action)
    Q_UNUSED(delaySeconds)
    return failure(tr("System-level scheduling is only implemented for macOS and Linux."));
#endif
}

PowerResult PowerOp::cancelScheduledPowerActions()
{
#ifdef Q_OS_MACOS
    const QString pidPath = QStringLiteral("/private/tmp/autopower-scheduled-power.pid");
    if (!QFile::exists(pidPath)) {
        return success();
    }

    const QString command = QStringLiteral(
        "if [ -f %1 ]; then /bin/kill \"$(/bin/cat %1)\" >/dev/null 2>&1 || true; /bin/rm -f %1; fi")
        .arg(shellQuoted(pidPath));
    return runAdministratorShellCommand(command, tr("Unable to cancel macOS power action"));
#elif defined(Q_OS_LINUX)
    if (m_linuxScheduledUnit.isEmpty()) {
        return success();
    }

    const QString systemctlPath = QStandardPaths::findExecutable(QStringLiteral("systemctl"));
    if (systemctlPath.isEmpty()) {
        return failure(tr("systemctl is required to cancel Linux power actions."));
    }

    QStringList arguments;
    arguments << systemctlPath << QStringLiteral("stop")
              << QStringLiteral("%1.timer").arg(m_linuxScheduledUnit)
              << QStringLiteral("%1.service").arg(m_linuxScheduledUnit);

    const PowerResult result = runPkexec(arguments, tr("Unable to cancel Linux power action"));
    if (result.success) {
        m_linuxScheduledUnit.clear();
    }
    return result;
#else
    return success();
#endif
}

bool PowerOp::supportsSleep() const
{
#ifdef Q_OS_WIN
    return true;
#elif defined(Q_OS_MACOS)
    return true;
#elif defined(Q_OS_LINUX)
    return login1CapabilityAllowed(QStringLiteral("CanSuspend"));
#else
    return false;
#endif
}

bool PowerOp::supportsHibernate() const
{
#ifdef Q_OS_WIN
    return true;
#elif defined(Q_OS_MACOS)
    return false;
#elif defined(Q_OS_LINUX)
    return login1CapabilityAllowed(QStringLiteral("CanHibernate"));
#else
    return false;
#endif
}
