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
        cerr << "创建进程快照失败!" << endl;
        return "";
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        cerr << "获取第一个进程信息失败!" << endl;
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
    // 检测鼠标是否左右键同时按下
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
    HWND hwndBroadcast = FindWindow(NULL, "屏幕广播");
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
    cout << R"(声明:本软件仅供学习使用，不得用于其他用途，否则后果自负!
严禁搬运，转载，否则后果自负!)";
    cout << "尝试提权......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "权限提升成功\n" << endl;
    } else {
        cout << "权限提升失败,部分功能可能会失效!\n" << endl;
    }

    cout << "屏幕分辨率:" << max_x << "x" << max_y << "\n";
    PID = getPID("StudentMain.exe");
    if (PID == -1)
        cerr << "获取极域PID失败, 请确保极域已经启动并重试!\n";
    else
        cout << "极域PID: " << PID << "\n";
    mythwarePath = GetProcessPath("StudentMain.exe");
    if (!mythwarePath.empty())
    {
        cout << "极域安装路径: " << mythwarePath << endl;
    }
    else
    {
        cerr << "未找到极域进程!\n";
    }
    system("start .\\keyboardProtect.exe");
    cout << "键盘防护已启动! \n";
    _sleep(1000);
    while (1)
    {
        pii pos = getCursorPos();
        int x = pos.first;
        int y = pos.second;
        cout << "x:" << x << " y:" << y << "  ";
        if (x <= 5 && y <= 5 && mouse_pressed())
        {
            cout << "左上-终止极域  ";
            killMythware();
            PID = getPID("StudentMain.exe");
            if (PID != -1)
                cout << "操作失败, 请重试!\n";
            else
                cout << "操作成功!\n";
        }
        if (x <= 5 && y >= max_y - 5 && mouse_pressed())
        {
            cout << "左下-挂起极域  ";
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                cout << "获取PID失败, 请重试!\n";
            else
                pauseMythware();
        }
        if (x >= max_x - 5 && y <= 5 && mouse_pressed())
        {
            cout << "右上-使广播窗口化  ";
            bool status = ToggleBroadcastWindow();
            if (status)
                cout << "操作成功! \n";
            else
                cout << "操作失败! \n";
            _sleep(700);
        }
        if (x >= max_x - 5 && y >= max_y - 5 && mouse_pressed())
        {
            cout << "右下-启动并重新获取极域进程PID  ";
            string command = "start \"" + mythwarePath + "\"";
            system(command.c_str());
            cout << "极域已经启动!";
            _sleep(1000);
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                cerr << "获取极域PID失败, 请重试!\n";
            else
                cout << "极域PID: " << PID << "\n";
        }
        else
            cout << "\n";
        _sleep(50);
    }
}
