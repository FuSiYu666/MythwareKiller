@echo off
setlocal enabledelayedexpansion

:: 询问用户输入版本号
set /p version=请输入版本号(x.x.x或x.x格式):

:: 创建目标目录
set target_dir=MythwareKiller-!version!
if not exist "!target_dir!" (
    mkdir "!target_dir!"
)

:: 定义源文件列表
set files=main.cpp keyboardProtect.cpp ToolBox.cpp

:: 编译每个源文件
for %%f in (%files%) do (
    set output_name=%%~nf.exe
    echo [编译中] 正在编译 %%f...
    g++ %%f -o !output_name! -O3 -Wall -g -std=c++17 -static
    
    if exist "!output_name!" (
        echo [优化中] 正在压缩 !output_name!...
        strip !output_name!
        upx !output_name!
        
        echo [复制中] 正在复制 !output_name! 到 !target_dir!...
        copy /y !output_name! "!target_dir!\" >nul
        
        :: 删除根目录下的临时exe文件
        echo [清理中] 删除临时文件 !output_name!...
        del /q "!output_name!" >nul 2>nul
    ) else (
        echo [错误] 编译 %%f 失败
    )
)
echo [复制中] 正在复制 pssuspend64.exe 到 !target_dir!...
copy pssuspend64.exe "!target_dir!"

echo [复制中] 正在复制 LICENSE 到 !target_dir!...
copy LICENSE "!target_dir!"

:: 使用Bandizip创建压缩文件
echo [压缩中] 正在创建压缩文件...


:: 创建ZIP文件（完全修正的语法）
echo [1/2] 正在创建 ZIP 压缩包...
bz a -y -l:9 -p: -t:zip "!target_dir!.zip" "!target_dir!\*"

:: 创建7z文件（完全修正的语法）
echo [2/2] 正在创建 7z 压缩包...
bz a -y -l:9 -p: -t:7z "!target_dir!.7z" "!target_dir!\*"

:: 计算并显示SHA-256哈希值
echo.
echo ============== 操作成功！ ==============
echo.

:: 显示文件详情
echo 文件详情：
echo.

:: 检查是否有certutil工具来计算SHA-256
where certutil >nul 2>nul
if errorlevel 1 (
    echo [警告] 无法计算SHA-256哈希值(certutil不可用)
    echo 生成文件：
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