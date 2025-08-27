@echo off
setlocal enabledelayedexpansion

:: ѯ���û�����汾��
set /p version=������汾��(x.x.x��x.x��ʽ):

:: ����Ŀ��Ŀ¼
set target_dir=MythwareKiller-!version!
if not exist "!target_dir!" (
    mkdir "!target_dir!"
)

:: ����Դ�ļ��б�
set files=main.cpp keyboardProtect.cpp ToolBox.cpp

:: ����ÿ��Դ�ļ�
for %%f in (%files%) do (
    set output_name=%%~nf.exe
    echo [������] ���ڱ��� %%f...
    g++ %%f -o !output_name! -O3 -Wall -g -std=c++17 -static
    
    if exist "!output_name!" (
        echo [�Ż���] ����ѹ�� !output_name!...
        strip !output_name!
        upx !output_name!
        
        echo [������] ���ڸ��� !output_name! �� !target_dir!...
        copy /y !output_name! "!target_dir!\" >nul
        
        :: ɾ����Ŀ¼�µ���ʱexe�ļ�
        echo [������] ɾ����ʱ�ļ� !output_name!...
        del /q "!output_name!" >nul 2>nul
    ) else (
        echo [����] ���� %%f ʧ��
    )
)
echo [������] ���ڸ��� pssuspend64.exe �� !target_dir!...
copy pssuspend64.exe "!target_dir!"

echo [������] ���ڸ��� LICENSE �� !target_dir!...
copy LICENSE "!target_dir!"

:: ʹ��Bandizip����ѹ���ļ�
echo [ѹ����] ���ڴ���ѹ���ļ�...


:: ����ZIP�ļ�����ȫ�������﷨��
echo [1/2] ���ڴ��� ZIP ѹ����...
bz a -y -l:9 -p: -t:zip "!target_dir!.zip" "!target_dir!\*"

:: ����7z�ļ�����ȫ�������﷨��
echo [2/2] ���ڴ��� 7z ѹ����...
bz a -y -l:9 -p: -t:7z "!target_dir!.7z" "!target_dir!\*"

:: ���㲢��ʾSHA-256��ϣֵ
echo.
echo ============== �����ɹ��� ==============
echo.

:: ��ʾ�ļ�����
echo �ļ����飺
echo.

:: ����Ƿ���certutil����������SHA-256
where certutil >nul 2>nul
if errorlevel 1 (
    echo [����] �޷�����SHA-256��ϣֵ(certutil������)
    echo �����ļ���
    echo   !target_dir!.zip
    echo   !target_dir!.7z
) else (
    echo 1) !target_dir!.zip
    for /f "skip=1 tokens=*" %%h in ('certutil -hashfile "!target_dir!.zip" SHA256 ^| find /v "hash" ^| find /v "CertUtil"') do (
        echo    SHA-256: %%h
    )
    echo.
    echo 2) !target_dir!.7z
    for /f "skip=1 tokens=*" %%h in ('certutil -hashfile "!target_dir!.7z" SHA256 ^| find /v "hash" ^| find /v "CertUtil"') do (
        echo    SHA-256: %%h
    )
)

echo.