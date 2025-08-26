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
BOOL SuspendProcess(DWORD dwProcessID, BOOL suspend) {
    NtSuspendProcess mNtSuspendProcess;
    NtResumeProcess mNtResumeProcess;
    HMODULE ntdll = GetModuleHandle("ntdll.dll");
    HANDLE handle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessID);
    if (suspend) {
        mNtSuspendProcess = (NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
        return mNtSuspendProcess(handle) == 0;
    } else {
        mNtResumeProcess = (NtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess");
        return mNtResumeProcess(handle) == 0;
    }
}

// ��־����������ָ����ʽ�����־��Ϣ
void logs(string message, bool isError = false) {
    // ��ȡ��ǰʱ��
    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    // ��ʽ��ʱ���
    char timestamp[9];
    sprintf(timestamp, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    cout << "[" << timestamp << "] ";
    // ����isError����������ɫ���ı�
    if (!isError) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "ERROR ";
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "SUCCESS ";
    }
    
    // �����־���ݲ��ָ�Ĭ����ɫ
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << message << endl;
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
    system("taskkill /f /im StudentMain.exe /t");
    system("taskkill /f /im MasterHelper.exe /t");
}

void pauseMythware()
{
    string command = ".\\pssuspend64.exe " + to_string(PID);
    system(command.c_str());
    
    // ���API���ù�����˫�ط���
    SuspendProcess(PID, TRUE);
}

void unblockNetwork()
{
    system("taskkill /f /im MasterHelper.exe /t");
    system("sc stop tdnetfilter");
    system("netsh advfirewall set allprofiles state off");
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
    cout << R"(����:���������ѧϰʹ�ã���������������;���������Ը�!
���λ��Github�ֿ�: FuSiYu666/MythwareKiller)" << endl;
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME))
    {
        logs("Ȩ�������ɹ�", true);
    }
    else
    {
        logs("Ȩ������ʧ��,���ֹ��ܿ��ܻ�ʧЧ!", false);
    }

    logs("��Ļ�ֱ���:" + to_string(max_x) + "x" + to_string(max_y), true);
    PID = getPID("StudentMain.exe");
    if (PID == -1)
        logs("��ȡ����PIDʧ��, ��ȷ�������Ѿ�����������!", false);
    else
        logs("����PID: " + to_string(PID), true);
    mythwarePath = GetProcessPath("StudentMain.exe");
    if (!mythwarePath.empty())
    {
        logs("����װ·��: " + mythwarePath, true);
    }
    else
    {
        logs("δ�ҵ��������!", false);
    }
    
    // ʹ��CreateProcess��̨����keyboardProtect.exe
    STARTUPINFO si = { sizeof(STARTUPINFO) };
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
        cout << "x:" << x << " y:" << y << "\n";
        if (x <= 5 && y <= 5 && mouse_pressed())
        {
            killMythware();
            PID = getPID("StudentMain.exe");
            if (PID != -1)
                logs("��ֹ����ʧ��, ������!", false);
            else
                logs("��ֹ����ɹ�!", true);
        }
        if (x <= 5 && y >= max_y - 5 && mouse_pressed())
        {
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                logs("������-��ȡPIDʧ��, ������!", false);
            else
            {
                pauseMythware();
                logs("������ɹ�!", true);
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
            string command = "start " + mythwarePath;
            system(command.c_str());
            logs("�����Ѿ�����!", true);
            _sleep(1000);
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                logs("��ȡ����PIDʧ��, ������!", false);
            else
                logs("����PID: " + to_string(PID), true);
        }
        _sleep(50);
    }
}