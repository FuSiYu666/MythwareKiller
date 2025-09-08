# 极域杀手
**本仓库使用GNU General Public License v3.0开源协议并附"未经特别事先书面许可，著作权人的姓名和其贡献者的姓名不得用于认可或推销源自该软件的产品"协议，详见LICENCE文件**

**声明:本软件仅供学习使用，不得用于其他用途，否则后果自负!**

我们强烈**不建议**您这么做, 这可能会使您的听课效率下降, 除非您确实希望摆脱教师的控制.

此外, 这还可能激怒您的教师, 对此我们**概不负责**.

## 版权声明与免责声明

1. **版权所有**：本软件及其源代码的版权归作者所有，受相关版权法和开源许可证保护。

2. **禁止冒名顶替**：未经作者明确书面许可，任何人不得声称本软件或其源代码为自己原创，不得在任何场合宣称自己是本软件的作者。

3. **禁止商业使用声明**：未经作者明确书面许可，禁止将本软件用于任何商业用途。

4. **免责声明**：本软件按"现状"提供，不提供任何形式的担保。作者不对因使用本软件而产生的任何直接、间接、偶然、特殊或后果性损害承担责任。

5. **知识产权保护**：作者保留对本软件及其源代码的所有权利。任何对本软件的修改和分发都必须遵守GNU General Public License v3.0的条款。

6. **法律追责**：对于任何侵犯作者权益、冒用作者名义或违反许可证条款的行为，作者保留追究法律责任的权利。
   
## 使用
 - 方法 1: 使用命令快捷运行
   1. 按下电脑Win+R，打开运行框
   2. 输入以下命令: ```bash powershell irm https://myth.fsy.qzz.io/start.ps1 | iex ``` 或 ```bash powershell irm https://myth.fsyfsyfsy.dpdns.org/start.ps1 | iex ```
   3. 等待脚本下载完毕并运行程序(程序会被下载到 `%UserProfile%\MythwareKiller` 目录下)

 - 方法 2: 手动下载运行
   1. 下载Release或者仓库源代码中的Mythware-x.x.x.7z/zip文件并解压
   2. 手动运行程序
 - 程序说明: 
   1. main.exe为主程序，功能如下:
   - 启动时自动获取极域进程PID，极域程序安装位置，屏幕分辨率，防键盘锁
   - 鼠标位于屏幕左上并左右键同时按下：终止极域
   - 鼠标位于屏幕左下并左右键同时按下：挂起极域
   - 鼠标位于屏幕右上并左右键同时按下：使全屏广播窗口化
   - 鼠标位于屏幕右下并左右键同时按下：启动并重新获取极域进程PID
   2. keyboardProtect.exe为负责防键盘锁的子程序
   3. pssuspend64.exe为用于挂起进程的子程序
   4. Toolbox.exe为集各种功能为一体的便携工具箱

## 编译
### 一、自动编译(推荐)-方法1

0. 准备条件：安装mingw64环境，安装upx工具，安装strip工具
1. 启动脚本compile.cmd
2. 编译完成后，所有可执行文件都会在 MythwareKillerCompiled 文件夹下

### 二、自动编译-方法2

0. 准备条件：安装mingw64环境，安装upx工具，安装strip工具，安装bandizip压缩工具，安装certutil工具
1. 启动脚本compile-zip.cmd并输入版本号
2. 编译完成后，所有可执行文件都会在 MythwareKiller-{版本号} 文件夹下，并且有压缩文件MythwareKiller-{版本号}.zip与MythwareKiller-{版本号}.7z

### 三、手动编译
使用mingw64环境编译，编译命令为：

```bash
g++ {程序名}.cpp -o {程序名} -O3 -Wall -g -std=c++17 -static
```

并使用以下命令压缩exe大小：

```bash
strip {文件名}.exe
upx {文件名}.exe
```


# Github Star History


<a href="https://www.star-history.com/#FuSiYu666/MythwareKiller&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=FuSiYu666/MythwareKiller&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=FuSiYu666/MythwareKiller&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=FuSiYu666/MythwareKiller&type=Date" />
 </picture>
</a>