@echo off
echo ��ȷ�����ڵ�������ȷ��װ��mingw64��upx��strip����
pause

echo ���ڱ���main.exe......
g++ main.cpp -o main -O3 -Wall -g -std=c++17 -static

echo ���ڱ���ToolBox.exe......
g++ ToolBox.cpp -o ToolBox -O3 -Wall -g -std=c++17 -static

echo ���ڱ���keyboardProtect.exe......
g++ keyboardProtect.cpp -o keyboardProtect -O3 -Wall -g -std=c++17 -static

echo ����ѹ��main.exe......
strip main.exe
upx main.exe

echo ����ѹ��ToolBox.exe......
strip ToolBox.exe
upx ToolBox.exe

echo ����ѹ��keyboardProtect.exe......
strip keyboardProtect.exe
upx keyboardProtect.exe

echo ���ڴ���MythwareKillerCompiledĿ¼
mkdir MythwareKillerCompiled

echo ���ڸ����ļ�......
copy main.exe MythwareKillerCompiled\main.exe
copy ToolBox.exe MythwareKillerCompiled\ToolBox.exe
copy keyboardProtect.exe MythwareKillerCompiled\keyboardProtect.exe
copy pssuspend64.exe MythwareKillerCompiled\pssuspend64.exe

echo ����������ʱ�ļ�......
del main.exe
del ToolBox.exe
del keyboardProtect.exe

echo �������!