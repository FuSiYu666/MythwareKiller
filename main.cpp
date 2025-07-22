#include <bits/stdc++.h>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <psapi.h>
#define pii pair<int, int>
using namespace std;
long PID;
int max_x = GetSystemMetrics(SM_CXSCREEN);
int max_y = GetSystemMetrics(SM_CYSCREEN);
#pragma comment(lib, "psapi.lib")
string mythwarePath = "";


BOOL IsRunAsAdministrator() {
BOOL fIsRunAsAdmin = FALSE;
PSID pAdministratorsGroup = NULL;
SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
return FALSE;
}
if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
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
    // �������Ƿ����Ҽ�ͬʱ����
    if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON))
    {
        return true;
    }
    return false;
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
�Ͻ����ˣ�ת�أ��������Ը�!)";
    cout << "������Ȩ......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "Ȩ�������ɹ�\n" << endl;
    } else {
        cout << "Ȩ������ʧ��,���ֹ��ܿ��ܻ�ʧЧ!\n" << endl;
    }

    cout << "��Ļ�ֱ���:" << max_x << "x" << max_y << "\n";
    PID = getPID("StudentMain.exe");
    if (PID == -1)
        cerr << "��ȡ����PIDʧ��, ��ȷ�������Ѿ�����������!\n";
    else
        cout << "����PID: " << PID << "\n";
    mythwarePath = GetProcessPath("StudentMain.exe");
    if (!mythwarePath.empty())
    {
        cout << "����װ·��: " << mythwarePath << endl;
    }
    else
    {
        cerr << "δ�ҵ��������!\n";
    }
    system("start .\\keyboardProtect.exe");
    cout << "���̷���������! \n";
    _sleep(1000);
    while (1)
    {
        pii pos = getCursorPos();
        int x = pos.first;
        int y = pos.second;
        cout << "x:" << x << " y:" << y << "  ";
        if (x <= 5 && y <= 5 && mouse_pressed())
        {
            cout << "����-��ֹ����  ";
            killMythware();
            PID = getPID("StudentMain.exe");
            if (PID != -1)
                cout << "����ʧ��, ������!\n";
            else
                cout << "�����ɹ�!\n";
        }
        if (x <= 5 && y >= max_y - 5 && mouse_pressed())
        {
            cout << "����-������  ";
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                cout << "��ȡPIDʧ��, ������!\n";
            else
                pauseMythware();
        }
        if (x >= max_x - 5 && y <= 5 && mouse_pressed())
        {
            cout << "����-ʹ�㲥���ڻ�  ";
            bool status = ToggleBroadcastWindow();
            if (status)
                cout << "�����ɹ�! \n";
            else
                cout << "����ʧ��! \n";
            _sleep(700);
        }
        if (x >= max_x - 5 && y >= max_y - 5 && mouse_pressed())
        {
            cout << "����-���������»�ȡ�������PID  ";
            string command = "start \"" + mythwarePath + "\"";
            system(command.c_str());
            cout << "�����Ѿ�����!";
            _sleep(1000);
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                cerr << "��ȡ����PIDʧ��, ������!\n";
            else
                cout << "����PID: " << PID << "\n";
        }
        else
            cout << "\n";
        _sleep(50);
    }
}
