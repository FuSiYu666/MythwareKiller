#include <windows.h>
#include <tchar.h>

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

int main()
{
    // ��������
    StartKeyboardUnlock();

    MessageBox(NULL,
               _T("���̽��������������ȷ������򽫼����ں�̨���С�"),
               _T("������̽�������"),
               MB_OK | MB_ICONINFORMATION);

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