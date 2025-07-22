#include <bits/stdc++.h>
#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <psapi.h>
using namespace std;
// 键盘钩子相关变量
HHOOK kbdHook;
HANDLE hKeybdThread = NULL;
DWORD dwThreadId = 0;

// 键盘钩子回调函数 - 直接拦截所有键盘输入
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // 直接拦截所有键盘输入
    if (nCode >= 0)
    {
        return 1; // 拦截按键
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// 键盘钩子线程函数 - 循环设置和取消钩子
DWORD WINAPI KeyHookThreadProc(LPVOID lpParameter)
{
    while (true)
    {
        // 设置低级键盘钩子
        kbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

        // 保持钩子25ms后取消
        Sleep(25);
        UnhookWindowsHookEx(kbdHook);
    }
    return 0;
}

// 欺骗极域的键盘驱动
bool BypassKeyboardDriver()
{
    // 打开极域的键盘驱动设备
    HANDLE hDevice = CreateFile(_T("\\\\.\\TDKeybd"),
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // 发送控制代码欺骗驱动
    BOOL bEnable = TRUE;
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(hDevice,
                                  0x220000,
                                  &bEnable,
                                  sizeof(bEnable),
                                  NULL,
                                  0,
                                  &bytesReturned,
                                  NULL);

    CloseHandle(hDevice);
    return result;
}

// 启动键盘解锁
void StartKeyboardUnlock()
{
    // 先尝试欺骗键盘驱动
    BypassKeyboardDriver();

    // 创建键盘钩子线程
    if (hKeybdThread == NULL)
    {
        hKeybdThread = CreateThread(NULL, 0, KeyHookThreadProc, NULL, 0, &dwThreadId);
    }
}

// 停止键盘解锁
void StopKeyboardUnlock()
{
    if (hKeybdThread)
    {
        TerminateThread(hKeybdThread, 0);
        CloseHandle(hKeybdThread);
        hKeybdThread = NULL;
    }

    // 确保钩子被取消
    if (kbdHook)
    {
        UnhookWindowsHookEx(kbdHook);
        kbdHook = NULL;
    }
}
bool EnablePrivileges(HANDLE hProcess, const char *pszPrivilegesName)
{
    HANDLE hToken = NULL;
    LUID luidValue;
    TOKEN_PRIVILEGES tokenPrivileges;
    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        return FALSE;
    }
    if (!LookupPrivilegeValue(NULL, pszPrivilegesName, &luidValue))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luidValue;
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    CloseHandle(hToken);
    return GetLastError() == ERROR_SUCCESS;
}
int main()
{
    cout << R"(声明:本软件仅供学习使用，不得用于其他用途，否则后果自负!
严禁搬运，转载，否则后果自负!)" << "\n";
    cout << "尝试提权......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "权限提升成功! \n" << endl;
    } else {
        cout << "权限提升失败,功能可能会失效!\n" << endl;
    }
    StartKeyboardUnlock();

    cout << "键盘解锁已启动!\n";

    // 保持程序运行
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 程序退出时停止解锁
    StopKeyboardUnlock();
    return 0;
}