@echo off
setlocal

rem  Build the PC simulator and launch it.
rem
rem      sim\build.bat            build, then launch the windowed simulator
rem      sim\build.bat build      build only, no window
rem
rem  Launching is the default because this is what gets run after every edit.
rem
rem  The one thing this exists for: the linker cannot replace dashboard_sim.exe
rem  while it is running, and make stops at that failure - so dashboard_shot.exe
rem  never gets rebuilt and quietly keeps running yesterday's code. That is a
rem  genuinely confusing way to lose an afternoon, so the running simulator is
rem  closed first.

set "SIMDIR=%~dp0"
set "BUILD=%SIMDIR%build"

rem  taskkill returns 0 when it killed something and 128 when there was nothing
rem  to kill, which is all the detection needed - no tasklist, no find.
rem
rem  Full paths on purpose. Run from Git Bash, a bare "find" resolves to Git's
rem  Unix find rather than the Windows one, and the check this replaced silently
rem  did nothing.
"%SystemRoot%\System32\taskkill.exe" /im dashboard_sim.exe /f >nul 2>&1
if not errorlevel 1 (
    echo Closed the running simulator so its executable can be replaced.
    rem Windows releases the file handle a moment after the process goes.
    "%SystemRoot%\System32\ping.exe" -n 2 127.0.0.1 >nul
)

if not exist "%BUILD%\CMakeCache.txt" (
    echo Configuring...
    cmake -S "%SIMDIR%." -B "%BUILD%"
    if errorlevel 1 (
        echo.
        echo Configure failed. A host GCC ^(TDM-GCC or any MinGW-w64^) and CMake
        echo both need to be on PATH.
        exit /b 1
    )
)

cmake --build "%BUILD%"
if errorlevel 1 (
    echo.
    echo Build failed - not launching.
    exit /b 1
)

echo.
echo   left / right arrow  = the two dashboard buttons
echo   both held for 1 s   = toggle the FPS overlay
echo.
echo   dashboard_shot.exe out.bmp ^<ms^> ^<page^>   headless screenshots

if /i "%~1"=="build" (
    echo.
    echo Built only, as asked.
    exit /b 0
)

echo.
start "" "%BUILD%\dashboard_sim.exe"
exit /b 0
