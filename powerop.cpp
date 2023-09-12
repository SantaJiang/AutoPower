#include "powerop.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include "powrprof.h"
#include "winuser.h"
#include "processthreadsapi.h"
#pragma comment(lib,"PowrProf.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Advapi32.lib")
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

PowerOp::PowerOp(QObject *parent)
    : QObject{parent}
{

}

void PowerOp::shutdown()
{

#ifdef Q_OS_WIN
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
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

    return;
}

void PowerOp::reboot()
{

#ifdef Q_OS_WIN
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
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

    return;
}

void PowerOp::hibernate()
{

#ifdef Q_OS_WIN
    // 让系统进入休眠 ..
    SetSuspendState(TRUE, TRUE, FALSE);
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

    return;
}

void PowerOp::sleep()
{

#ifdef Q_OS_WIN
    // 让系统进入睡眠..
    SetSuspendState(FALSE, TRUE, FALSE);
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

    return;
}

void PowerOp::closeMonitor()
{

#ifdef Q_OS_WIN
    //关闭显示器..
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) 2);
#elif Q_OS_MACOS
#elif Q_OS_LINUX
#endif

    return;
}
