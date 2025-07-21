#include <windows.h>
#include <tchar.h>

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

int main()
{
    // 启动解锁
    StartKeyboardUnlock();

    MessageBox(NULL,
               _T("键盘解锁已启动。点击确定后程序将继续在后台运行。"),
               _T("极域键盘解锁工具"),
               MB_OK | MB_ICONINFORMATION);

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