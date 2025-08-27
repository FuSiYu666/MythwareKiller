/*
 * ����ɱ�� (Mythware Killer) - ������
 * ��Ȩ���� (C) 2025 FSY
 *
 * ����������������������Ը��������������ᷢ����GNUͨ�ù������֤���������ѡ��ģ��κθ��߰汾���������·ַ���/���޸�����
 *
 * ������ķַ���ϣ�����ܷ������ã������ṩ�κε���������û�������Ի��ض���;�����Եİ�ʾ�������й���ϸ��Ϣ�������GNUͨ�ù������֤��
 *
 * ��Ӧ���Ѿ��յ���GNUͨ�ù������֤�ĸ��������û�У���μ�<http://www.gnu.org/licenses/>��
 *
 * ��Ҫ������
 * 1. ���������ѧϰ���о�Ŀ��ʹ�ã����������κ���ҵ��;��
 * 2. δ��������ȷ������ɣ��κ��˲������Ʊ��������Դ����Ϊ�Լ�ԭ����
 * 3. ��ֹ���κγ��������Լ��Ǳ���������ߡ�
 * 4. ���߲�����ʹ�ñ�������������κ�ֱ�ӡ���ӡ�żȻ������������𺦳е����Ρ�
 */

#include <bits/stdc++.h>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <psapi.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

#define pii pair<int, int>
using namespace std;
long PID;
int max_x = GetSystemMetrics(SM_CXSCREEN);
int max_y = GetSystemMetrics(SM_CYSCREEN);
#pragma comment(lib, "psapi.lib")
string mythwarePath = "";

typedef NTSTATUS(NTSYSAPI NTAPI *NtSuspendProcess)(IN HANDLE Process);
typedef NTSTATUS(NTSYSAPI NTAPI *NtResumeProcess)(IN HANDLE Process);
BOOL SuspendProcess(DWORD dwProcessID, BOOL suspend)
{
    NtSuspendProcess mNtSuspendProcess;
    NtResumeProcess mNtResumeProcess;
    HMODULE ntdll = GetModuleHandle("ntdll.dll");
    HANDLE handle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessID);
    if (suspend)
    {
        mNtSuspendProcess = (NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
        return mNtSuspendProcess(handle) == 0;
    }
    else
    {
        mNtResumeProcess = (NtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess");
        return mNtResumeProcess(handle) == 0;
    }
}
int logs_len = 15;
void sys_quiet(string command)
{
    command += ">nul 2>&1";
    system(command.c_str());
}
void setCursorPosition(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hOut, coord);
}

// ���ָ���е�����
void clearLine(int lineNumber)
{
    setCursorPosition(0, lineNumber);

    // ��ȡ����̨���
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.dwSize.X;

    // �ÿո񸲸�����
    std::cout << std::string(consoleWidth, ' ');

    // ������ƻ�����
    setCursorPosition(0, lineNumber);
}

// ��ָ�����������
void writeToLine(int lineNumber, const std::string &content)
{
    clearLine(lineNumber);
    std::cout << content;
}

// ���ع�������˸
void hideCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

// �����־
void logs(string message, bool isError = false)
{
    int lineNumber = ++logs_len;
    // ����ԭʼ���λ��
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    COORD originalPos = csbi.dwCursorPosition;

    // �ƶ���Ŀ����
    setCursorPosition(0, lineNumber);

    // ��ȡ��ǰʱ��
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // ��ʽ��ʱ���
    cout << "["
         << setw(2) << setfill('0') << ltm->tm_hour << ":"
         << setw(2) << setfill('0') << ltm->tm_min << ":"
         << setw(2) << setfill('0') << ltm->tm_sec << "] ";

    // ������־������ɫ���ı�
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!isError)
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "ERROR ";
    }
    else
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "SUCCESS ";
    }

    // �����־���ݲ��ָ�Ĭ����ɫ
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << message;

    // �����ʣ�ಿ��
    CONSOLE_SCREEN_BUFFER_INFO newCsbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &newCsbi);
    int remainingWidth = newCsbi.dwSize.X - newCsbi.dwCursorPosition.X;
    if (remainingWidth > 0)
    {
        cout << string(remainingWidth, ' ');
    }

    // �ָ�ԭʼ���λ��
    setCursorPosition(originalPos.X, originalPos.Y);
}

BOOL IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
    {
        return FALSE;
    }
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        FreeSid(pAdministratorsGroup);
        return FALSE;
    }
    FreeSid(pAdministratorsGroup);
    return fIsRunAsAdmin;
}

string GetProcessPath(const char *processName)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cerr << "�������̿���ʧ��!" << endl;
        return "";
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        cerr << "��ȡ��һ��������Ϣʧ��!" << endl;
        CloseHandle(hProcessSnap);
        return "";
    }

    do
    {
        if (_stricmp(pe32.szExeFile, processName) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL)
            {
                char path[MAX_PATH];
                if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH))
                {
                    CloseHandle(hProcess);
                    CloseHandle(hProcessSnap);
                    return path;
                }
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return "";
}

HWND GetForegroundWindowHandle()
{
    return GetForegroundWindow();
}

pair<int, int> getCursorPos()
{
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}
long getPID(string name)
{
    string command = "tasklist | findstr \"" + name + "\" > s.tmp";
    system(command.data());
    ifstream in("s.tmp");
    int c = in.get();
    if (c == EOF)
    {
        in.close();
        system("del s.tmp");
        return -1;
    }
    string i;
    int word = 0;
    while (in >> i)
    {
        if (word == 1)
        {
            break;
        }
        word++;
    }
    in.close();
    system("del s.tmp");
    long pid = atoi(i.c_str());
    return pid;
}

bool mouse_pressed()
{
    return GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON);
}
void killMythware()
{
    sys_quiet("taskkill /f /im StudentMain.exe /t");
    sys_quiet("taskkill /f /im MasterHelper.exe /t");
}

void pauseMythware()
{
    string command = ".\\pssuspend64.exe " + to_string(PID);
    sys_quiet(command.c_str());

    // ���API���ù�����˫�ط���
    SuspendProcess(PID, TRUE);
}

void unblockNetwork()
{
    sys_quiet("taskkill /f /im MasterHelper.exe /t");
    sys_quiet("sc stop tdnetfilter");
    sys_quiet("netsh advfirewall set allprofiles state off");
}

void RestoreWindowFreedom(HWND hWnd)
{
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
    style |= WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongPtr(hWnd, GWL_STYLE, style);
    LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
    SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

bool ToggleBroadcastWindow()
{
    HWND hwndBroadcast = FindWindow(NULL, "��Ļ�㲥");
    if (hwndBroadcast == NULL)
    {
        return false;
    }

    PostMessage(hwndBroadcast, WM_COMMAND, WPARAM((BN_CLICKED << 16) | 1004), NULL);

    return true;
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
    hideCursor();
    writeToLine(0, "");
    writeToLine(1, "   MythwareKiller-2.0.0");
    writeToLine(2, "");
    writeToLine(3, "=================================================");
    writeToLine(4, "�����λ��Github�ֿ�FuSiYu666/MythwareKiller, ��ȫ��Դ���ʹ��");
    writeToLine(5, "���������ѧϰʹ�ã���������������;���������Ը���");
    writeToLine(6, "ʹ�ñ����Ĭ����ͬ��LICENSE�ļ��µ�GPL-3.0 licenseЭ��");
    writeToLine(7, "Copyright (C) 2025 FSY");
    writeToLine(8, "=================================================");
    writeToLine(9, "��ǰ�����Ƿ���������: ��ȡ��");
    writeToLine(10, "��ǰ�������PID: ��ȡ��");
    writeToLine(11, "��ǰ����װ·��: ��ȡ��");
    writeToLine(12, "��ǰ��Ļ�ֱ���: ��ȡ��");
    writeToLine(13, "��ǰ���λ��: x:��ȡ�� y:��ȡ��");
    writeToLine(14, "=================================================");
    writeToLine(15, "��־:");
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME))
    {
        logs("Ȩ�������ɹ�", true);
    }
    else
    {
        logs("Ȩ������ʧ��,���ֹ��ܿ��ܻ�ʧЧ!", false);
    }

    writeToLine(12, "��ǰ��Ļ�ֱ���: " + to_string(max_x) + "x" + to_string(max_y));
    logs("��Ļ�ֱ���:" + to_string(max_x) + "x" + to_string(max_y), true);

    PID = getPID("StudentMain.exe");

    if (PID == -1)
    {
        logs("��ȡ����PIDʧ��, ��ȷ�������Ѿ�����������!", false);
        writeToLine(9, "��ǰ�����Ƿ��������У���");
        writeToLine(10, "��ǰ�������PID: ��");
    }
    else
    {
        logs("����PID: " + to_string(PID), true);
        writeToLine(9, "��ǰ�����Ƿ��������У���");
        writeToLine(10, "��ǰ�������PID: " + to_string(PID));
    }
    mythwarePath = GetProcessPath("StudentMain.exe");
    if (!mythwarePath.empty())
    {
        logs("����װ·��: " + mythwarePath, true);
        writeToLine(11, "��ǰ����װ·��: " + mythwarePath);
    }
    else
    {
        logs("δ�ҵ��������!", false);
        writeToLine(11, "��ǰ����װ·��: ��ȡʧ��");
    }

    // ʹ��CreateProcess��̨����keyboardProtect.exe
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    CreateProcess(NULL, ".\\keyboardProtect.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    _sleep(1000);
    while (1)
    {
        pii pos = getCursorPos();
        int x = pos.first;
        int y = pos.second;
        writeToLine(13, "��ǰ���λ��: x:" + to_string(x) + " y:" + to_string(y));
        if (x <= 5 && y <= 5 && mouse_pressed())
        {
            killMythware();
            PID = getPID("StudentMain.exe");
            if (PID != -1)
            {
                writeToLine(10, "��ǰ�������PID: " + to_string(PID));
                writeToLine(9, "��ǰ�����Ƿ��������У���");
                logs("��ֹ����ʧ��, ������!", false);
            }
            else
            {
                writeToLine(10, "��ǰ�������PID: ��");
                writeToLine(9, "��ǰ�����Ƿ��������У���");
                logs("��ֹ����ɹ�!", true);
            }
        }
        if (x <= 5 && y >= max_y - 5 && mouse_pressed())
        {
            PID = getPID("StudentMain.exe");
            if (PID == -1)
            {
                logs("������-��ȡPIDʧ��, ������!", false);
                writeToLine(10, "��ǰ�������PID: ��");
                writeToLine(9, "��ǰ�����Ƿ��������У���");
            }
            else
            {
                pauseMythware();
                logs("������ɹ�!", true);
                writeToLine(10, "��ǰ�������PID: " + to_string(PID));
                writeToLine(9, "��ǰ�����Ƿ��������У�����");
            }
        }
        if (x >= max_x - 5 && y <= 5 && mouse_pressed())
        {
            bool status = ToggleBroadcastWindow();
            if (status)
                logs("ʹ�㲥���ڻ��ɹ�!", true);
            else
                logs("ʹ�㲥���ڻ�ʧ��!", false);
            _sleep(700);
        }
        if (x >= max_x - 5 && y >= max_y - 5 && mouse_pressed())
        {
            string command = "start \"\" \"" + mythwarePath + "\"";
            sys_quiet(command.c_str());
            _sleep(1500);
            PID = getPID("StudentMain.exe");
            if (PID == -1)
            {
                writeToLine(10, "��ǰ�������PID: ��");
                writeToLine(9, "��ǰ�����Ƿ��������У���");
                logs("��������ʧ��, ������!", false);
            }
            else
            {
                writeToLine(10, "��ǰ�������PID: " + to_string(PID));
                writeToLine(9, "��ǰ�����Ƿ��������У���");
                logs("���������ɹ�! PID: " + to_string(PID), true);
            }
        }
        _sleep(50);
    }
}