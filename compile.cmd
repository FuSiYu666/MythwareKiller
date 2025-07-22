@echo off
echo 请确保你在电脑上正确安装了mingw64，upx和strip工具
pause

echo 正在编译main.exe......
g++ main.cpp -o main -O3 -Wall -g -std=c++17 -static

echo 正在编译ToolBox.exe......
g++ ToolBox.cpp -o ToolBox -O3 -Wall -g -std=c++17 -static

echo 正在编译keyboardProtect.exe......
g++ keyboardProtect.cpp -o keyboardProtect -O3 -Wall -g -std=c++17 -static

echo 正在压缩main.exe......
strip main.exe
upx main.exe

echo 正在压缩ToolBox.exe......
strip ToolBox.exe
upx ToolBox.exe

echo 正在压缩keyboardProtect.exe......
strip keyboardProtect.exe
upx keyboardProtect.exe

echo 正在创建MythwareKillerCompiled目录
mkdir MythwareKillerCompiled

echo 正在复制文件......
copy main.exe MythwareKillerCompiled\main.exe
copy ToolBox.exe MythwareKillerCompiled\ToolBox.exe
copy keyboardProtect.exe MythwareKillerCompiled\keyboardProtect.exe
copy pssuspend64.exe MythwareKillerCompiled\pssuspend64.exe

echo 正在清理临时文件......
del main.exe
del ToolBox.exe
del keyboardProtect.exe

echo 操作完成!