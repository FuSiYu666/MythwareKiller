#include <bits/stdc++.h>
#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <psapi.h>
using namespace std;
// ���̹�����ر���
HHOOK kbdHook;
HANDLE hKeybdThread = NULL;
DWORD dwThreadId = 0;

// ���̹��ӻص����� - ֱ���������м�������
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // ֱ���������м�������
    if (nCode >= 0)
    {
        return 1; // ���ذ���
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ���̹����̺߳��� - ѭ�����ú�ȡ������
DWORD WINAPI KeyHookThreadProc(LPVOID lpParameter)
{
    while (true)
    {
        // ���õͼ����̹���
        kbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

        // ���ֹ���25ms��ȡ��
        Sleep(25);
        UnhookWindowsHookEx(kbdHook);
    }
    return 0;
}

// ��ƭ����ļ�������
bool BypassKeyboardDriver()
{
    // �򿪼���ļ��������豸
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

    // ���Ϳ��ƴ�����ƭ����
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

// �������̽���
void StartKeyboardUnlock()
{
    // �ȳ�����ƭ��������
    BypassKeyboardDriver();

    // �������̹����߳�
    if (hKeybdThread == NULL)
    {
        hKeybdThread = CreateThread(NULL, 0, KeyHookThreadProc, NULL, 0, &dwThreadId);
    }
}

// ֹͣ���̽���
void StopKeyboardUnlock()
{
    if (hKeybdThread)
    {
        TerminateThread(hKeybdThread, 0);
        CloseHandle(hKeybdThread);
        hKeybdThread = NULL;
    }

    // ȷ�����ӱ�ȡ��
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
    cout << R"(����:���������ѧϰʹ�ã���������������;���������Ը�!
�Ͻ����ˣ�ת�أ��������Ը�!)" << "\n";
    cout << "������Ȩ......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "Ȩ�������ɹ�! \n" << endl;
    } else {
        cout << "Ȩ������ʧ��,���ܿ��ܻ�ʧЧ!\n" << endl;
    }
    StartKeyboardUnlock();

    cout << "���̽���������!\n";

    // ���ֳ�������
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // �����˳�ʱֹͣ����
    StopKeyboardUnlock();
    return 0;
}