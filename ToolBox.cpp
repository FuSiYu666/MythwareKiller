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
    cout << "������Ȩ......\n";
    if (EnablePrivileges(GetCurrentProcess(), SE_SHUTDOWN_NAME)) {
        cout << "Ȩ�������ɹ�! \n" << endl;
    } else {
        cout << "Ȩ������ʧ��,���ܿ��ܻ�ʧЧ!\n" << endl;
    }
    while (1)
    {
        system("cls");
        cout << R"(����:���������ѧϰʹ�ã���������������;���������Ը�!
���λ��Github�ֿ�: FuSiYu666/MythwareKiller)" << endl;
        cout << "����ȫ�ܹ�����\n";
        cout << "1. ���U������\n";
        cout << "2. �����������\n";
        cout << "3. �����������\n";
        cout << "4. ���������\n";
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
                cerr << "��ȡ����PIDʧ��, ��ȷ�������Ѿ�����������!\n";
            else
            {
                cout << "PID: " << PID << endl;
                string command = ".\\pssuspend64.exe " + to_string(PID);
                system(command.c_str());
            }
        }
        cout << "�������!\n";
        _sleep(2000);
    }
    return 0;
}