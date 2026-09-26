@echo off
chcp 936>nul
setlocal enabledelayedexpansion

rem ============================================================
rem  LyraHy 一键编译脚本
rem
rem  双击 = 编译编辑器（LyraEditor / Development）。
rem  也可以拖到命令行里带参数用：
rem
rem    BuildLyra.bat            编译 LyraEditor  Development
rem    BuildLyra.bat game       编译 LyraGame     Development
rem    BuildLyra.bat shipping   编译 LyraGame     Shipping（打包用）
rem    BuildLyra.bat fixlib     修复损坏/缺失的 .lib（推荐在 LNK1136 时用）
rem    BuildLyra.bat clean      清空中间产物后全量重编译（很慢，慎用）
rem
rem  两个关键点：
rem   1. -NoUBA 是必须的。UE 5.6 默认启用 UBA（Unreal Build Accelerator），
rem      在本机会大量报 "Access is denied" 和 "LNK1136 无效或损坏的文件"。
rem   2. 不要只删 .lib 而不重建。实测 UBT 自己调用 link.exe 重建 .lib 会
rem      失败（返回 1 且无任何输出），但手动调用 link.exe 是成功的。
rem      所以 fixlib 做的是「删除 + 立刻手工重建」的闭环，不会把工程卡死。
rem ============================================================

rem ================= 配置区（换机器时改这里）=================
set "ENGINE=D:\UE_5.6"
set "UPROJECT=D:\ue_texiao\LyraHy\LyraHy.uproject"
set "PLATFORM=Win64"

rem link.exe 路径。若升级 VS 后失效，去
rem %LOCALAPPDATA%\UnrealBuildTool\Log.txt 里搜 "link.exe /LIB @"，
rem 把找到的实际路径填到这里。
set "LINKEXE=D:\cpp\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64\link.exe"

set "CONFIG=Development"
set "TARGET=LyraEditor"

rem ================= 解析参数 =================
set "MODE=%~1"
if "%MODE%"=="" set "MODE=editor"

if /i "%MODE%"=="editor"   set "TARGET=LyraEditor" & set "CONFIG=Development"
if /i "%MODE%"=="game"     set "TARGET=LyraGame"   & set "CONFIG=Development"
if /i "%MODE%"=="shipping" set "TARGET=LyraGame"   & set "CONFIG=Shipping"
if /i "%MODE%"=="clean"    set "TARGET=LyraEditor" & set "CONFIG=Development"

rem ================= 路径校验 =================
if not exist "%ENGINE%\Engine\Build\BatchFiles\Build.bat" (
    echo.
    echo [错误] 找不到引擎构建脚本：
    echo        %ENGINE%\Engine\Build\BatchFiles\Build.bat
    echo        请修改本脚本开头的 ENGINE 变量。
    goto :fail
)
if not exist "%UPROJECT%" (
    echo.
    echo [错误] 找不到工程文件：%UPROJECT%
    echo        请修改本脚本开头的 UPROJECT 变量。
    goto :fail
)

rem ================= 修复损坏的 .lib =================
if /i "%MODE%"=="fixlib" (
    if not exist "%LINKEXE%" (
        echo.
        echo [错误] 找不到 link.exe：%LINKEXE%
        echo        请打开 %%LOCALAPPDATA%%\UnrealBuildTool\Log.txt
        echo        搜索 "link.exe /LIB @"，把实际路径填到本脚本的 LINKEXE 变量。
        goto :fail
    )
    echo.
    echo [修复] 1/2 删除 Intermediate 下损坏的 .lib / .exp ...
    del /s /q "%~dp0Intermediate\*.lib" >nul 2>nul
    del /s /q "%~dp0Intermediate\*.exp" >nul 2>nul
    for /d /r "%~dp0Plugins" %%D in (Intermediate) do (
        if exist "%%D" (
            del /s /q "%%D\*.lib" >nul 2>nul
            del /s /q "%%D\*.exp" >nul 2>nul
        )
    )
    echo [修复] 2/2 按 .rsp 重新生成 .lib ...
    set "CNT=0"
    for /r "%~dp0Intermediate" %%R in (*.lib.rsp) do (
        "%LINKEXE%" /LIB @"%%R" >nul 2>nul
        set /a CNT+=1
    )
    for /r "%~dp0Plugins" %%R in (*.lib.rsp) do (
        "%LINKEXE%" /LIB @"%%R" >nul 2>nul
        set /a CNT+=1
    )
    echo [完成] 已重建 !CNT! 个 .lib，现在可以重新编译了。
    goto :done
)

rem ================= 全量清理（可选）=================
if /i "%MODE%"=="clean" (
    echo.
    echo [警告] 即将清空 Binaries 和 Intermediate，之后会全量重编译，耗时很久。
    echo        按任意键继续，或直接关闭窗口取消。
    pause >nul
    echo [清理] 删除 Binaries ...
    if exist "%~dp0Binaries" rd /s /q "%~dp0Binaries"
    echo [清理] 删除 Intermediate ...
    if exist "%~dp0Intermediate" rd /s /q "%~dp0Intermediate"
    for /d /r "%~dp0Plugins" %%D in (Binaries Intermediate) do (
        if exist "%%D" rd /s /q "%%D"
    )
    echo [清理] 完成，开始编译。
)

rem ================= 检查编辑器是否占用文件 =================
tasklist /fi "imagename eq UnrealEditor.exe" 2>nul | find /i "UnrealEditor.exe" >nul
if "%ERRORLEVEL%"=="0" (
    echo.
    echo [提示] 检测到 Unreal Editor 正在运行，它可能锁住 dll / pdb 导致链接失败。
    echo        如果下面报 LNK1136 或 Access is denied，请先关闭编辑器再重试。
    echo.
)

rem ================= 开始编译 =================
echo.
echo ============================================================
echo  目标：%TARGET%    平台：%PLATFORM%    配置：%CONFIG%
echo  工程：%UPROJECT%
echo  模式：已启用 -NoUBA（规避 UBA 的文件占用问题）
echo ============================================================
echo.

call "%ENGINE%\Engine\Build\BatchFiles\Build.bat" %TARGET% %PLATFORM% %CONFIG% -Project="%UPROJECT%" -WaitMutex -NoUBA

if errorlevel 1 goto :fail

echo.
echo [成功] 编译通过。
goto :done

:fail
echo.
echo ============================================================
echo [失败] 编译未通过。按以下顺序排查：
echo   1. 看上方 MSVC 报的 error Cxxxx / LNKxxxx，那是真正的代码错误。
echo   2. 若大量出现 LNK1136「无效或损坏的文件」或 Access is denied：
echo      先关闭 UE 编辑器，再运行 BuildLyra.bat fixlib，然后重新编译。
echo   3. 若改过大量头文件仍报奇怪的链接错，用 BuildLyra.bat clean 全量重编。
echo ============================================================
endlocal
exit /b 1

:done
endlocal
exit /b 0
