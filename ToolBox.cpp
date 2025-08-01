#include <bits/stdc++.h>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <psapi.h>
using namespace std;

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
    cout << "尝试提权......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "权限提升成功! \n" << endl;
    } else {
        cout << "权限提升失败,功能可能会失效!\n" << endl;
    }
    while (1)
    {
        system("cls");
        cout << R"(声明:本软件仅供学习使用，不得用于其他用途，否则后果自负!
软件位于Github仓库: FuSiYu666/MythwareKiller)" << endl;
        cout << "极域全能工具箱\n";
        cout << "1. 解除U盘限制\n";
        cout << "2. 结束极域进程\n";
        cout << "3. 解除网络限制\n";
        cout << "4. 挂起极域进程\n";
        int n;
        cin >> n;
        if (n == 1)
        {
            system("sc stop TDFileFilter");
            system("sc delete TDFileFilter");

            system("sc stop TDFileFilter");
            system("sc delete TDFileFilter");
        }
        if (n == 2)
        {
            system("taskkill /f /im StudentMain.exe /t");
            system("taskkill /f /im MasterHelper.exe /t");

            system("taskkill /f /im StudentMain.exe /t");
            system("taskkill /f /im MasterHelper.exe /t");
        }
        if (n == 3)
        {
            system("taskkill /f /im MasterHelper.exe /t");
            system("sc stop tdnetfilter");
            system("netsh advfirewall set allprofiles state off");

            system("taskkill /f /im MasterHelper.exe /t");
            system("sc stop tdnetfilter");
            system("netsh advfirewall set allprofiles state off");
        }
        if (n == 4)
        {
            int PID = getPID("StudentMain.exe");
            if (PID == -1)
                cerr << "获取极域PID失败, 请确保极域已经启动并重试!\n";
            else
            {
                cout << "PID: " << PID << endl;
                string command = ".\\pssuspend64.exe " + to_string(PID);
                system(command.c_str());
            }
        }
        cout << "操作完成!\n";
        _sleep(2000);
    }
    return 0;
}