/*
 * 极域杀手 (Mythware Killer) - 键盘保护模块
 * 版权所有 (C) 2025 FSY
 *
 * 本程序是自由软件：您可以根据自由软件基金会发布的GNU通用公共许可证第三版或（您选择的）任何更高版本的条款重新分发和/或修改它。
 *
 * 本程序的分发是希望它能发挥作用，但不提供任何担保；甚至没有适销性或特定用途适用性的暗示担保。有关详细信息，请参阅GNU通用公共许可证。
 *
 * 您应该已经收到了GNU通用公共许可证的副本。如果没有，请参见<http://www.gnu.org/licenses/>。
 *
 * 重要声明：
 * 1. 本软件仅供学习和研究目的使用，不得用于任何商业用途。
 * 2. 未经作者明确书面许可，任何人不得声称本软件或其源代码为自己原创。
 * 3. 禁止在任何场合宣称自己是本软件的作者。
 * 4. 作者不对因使用本软件而产生的任何直接、间接、偶然、特殊或后果性损害承担责任。
 */
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
软件位于Github仓库: FuSiYu666/MythwareKiller)" << endl;
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