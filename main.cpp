/*
 * 极域杀手 (Mythware Killer) - 主程序
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

// 日志函数，按照指定格式输出日志信息
void logs(string message, bool isError = false) {
    // 获取当前时间
    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    // 格式化时间戳
    char timestamp[9];
    sprintf(timestamp, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    cout << "[" << timestamp << "] ";
    // 根据isError参数设置颜色和文本
    if (!isError) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "ERROR ";
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "SUCCESS ";
    }
    
    // 输出日志内容并恢复默认颜色
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
    
    // 添加API调用挂起极域，双重方案
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
软件位于Github仓库: FuSiYu666/MythwareKiller)" << endl;
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME))
    {
        logs("权限提升成功", true);
    }
    else
    {
        logs("权限提升失败,部分功能可能会失效!", false);
    }

    logs("屏幕分辨率:" + to_string(max_x) + "x" + to_string(max_y), true);
    PID = getPID("StudentMain.exe");
    if (PID == -1)
        logs("获取极域PID失败, 请确保极域已经启动并重试!", false);
    else
        logs("极域PID: " + to_string(PID), true);
    mythwarePath = GetProcessPath("StudentMain.exe");
    if (!mythwarePath.empty())
    {
        logs("极域安装路径: " + mythwarePath, true);
    }
    else
    {
        logs("未找到极域进程!", false);
    }
    
    // 使用CreateProcess后台运行keyboardProtect.exe
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
                logs("终止极域失败, 请重试!", false);
            else
                logs("终止极域成功!", true);
        }
        if (x <= 5 && y >= max_y - 5 && mouse_pressed())
        {
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                logs("挂起极域-获取PID失败, 请重试!", false);
            else
            {
                pauseMythware();
                logs("挂起极域成功!", true);
            }
        }
        if (x >= max_x - 5 && y <= 5 && mouse_pressed())
        {
            bool status = ToggleBroadcastWindow();
            if (status)
                logs("使广播窗口化成功!", true);
            else
                logs("使广播窗口化失败!", false);
            _sleep(700);
        }
        if (x >= max_x - 5 && y >= max_y - 5 && mouse_pressed())
        {
            string command = "start " + mythwarePath;
            system(command.c_str());
            logs("极域已经启动!", true);
            _sleep(1000);
            PID = getPID("StudentMain.exe");
            if (PID == -1)
                logs("获取极域PID失败, 请重试!", false);
            else
                logs("极域PID: " + to_string(PID), true);
        }
        _sleep(50);
    }
}