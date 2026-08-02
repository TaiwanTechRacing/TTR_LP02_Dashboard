@echo off
setlocal

rem  Regenerate Core\User\ui from the EEZ project, without opening the editor.
rem
rem      tools\eez_export.bat
rem
rem  EEZ Studio takes --build-project and quits when it is done. It also skips
rem  the single-instance lock, so this works with the editor already open.
rem
rem  Two things to know:
rem
rem  1. A headless build DELETES the generated font .c files - font rendering
rem     seems to need the editor's own environment. They are restored from git
rem     afterwards, which is correct as long as no font changed. If you did
rem     change a font, export from the editor instead.
rem
rem  2. An open editor holds the project in memory and will write its copy back
rem     over any edit made to the .eez-project file on disk. Close it, or
rem     reopen the project, before editing that file outside the editor.

set "REPO=%~dp0.."
set "EEZ=%LOCALAPPDATA%\Programs\eezstudio\EEZ Studio.exe"
set "PROJECT=%REPO%\dashboard_layout\dashboard_layout.eez-project"

if not exist "%EEZ%" (
    echo Cannot find EEZ Studio at:
    echo   %EEZ%
    exit /b 1
)

echo Building %PROJECT%
"%EEZ%" --build-project "%PROJECT%"
if errorlevel 1 (
    echo.
    echo EEZ Studio reported a failure.
    exit /b 1
)

rem The fonts it just deleted. Restoring them is a no-op when they are already
rem in the tree unchanged, and it is what makes the headless path usable.
pushd "%REPO%"
git checkout -- Core/User/ui/ui_font_*.c 2>nul
popd

echo.
echo Regenerated Core\User\ui, fonts restored from git.
echo If you changed a font in the editor, export from the editor instead -
echo this path cannot rebuild them.
exit /b 0
