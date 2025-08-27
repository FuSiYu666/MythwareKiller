/*
 * 极域杀手 (Mythware Killer) - 工具箱模块
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
// 日志函数，按照指定格式输出日志信息
void logs(string message, bool isError = false)
{
    // 获取当前时间
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // 格式化时间戳
    char timestamp[9];
    sprintf(timestamp, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    cout << "[" << timestamp << "] ";
    // 根据isError参数设置颜色和文本
    if (!isError)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "ERROR ";
    }
    else
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "SUCCESS ";
    }

    // 输出日志内容并恢复默认颜色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << message << endl;
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
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME))
    {
        logs("权限提升成功!", true);
    }
    else
    {
        logs("权限提升失败,功能可能会失效!", false);
    }
    while (1)
    {
        system("cls");
        cout << R"(声明:本软件仅供学习使用，不得用于其他用途，否则后果自负!
软件位于Github仓库: FuSiYu666/MythwareKiller)"
             << endl;
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
                logs("获取极域PID失败, 请确保极域已经启动并重试!", false);
            else
            {
                cout << "PID: " << PID << endl;
                string command = ".\\pssuspend64.exe " + to_string(PID);
                system(command.c_str());
            }
        }
        logs("操作完成!", true);
        _sleep(2000);
    }
    return 0;
}