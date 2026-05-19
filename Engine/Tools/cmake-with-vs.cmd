@echo off
setlocal EnableExtensions

rem Chạy cmake trong môi trường MSVC (VsDevCmd) để CMake probe được STL include paths.
set "_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%_VSWHERE%" goto :vswhere_missing

for /f "usebackq delims=" %%I in (`"%_VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "_VSINSTALL=%%I"
if not defined _VSINSTALL goto :vs_missing

call "%_VSINSTALL%\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 >nul
if errorlevel 1 goto :vsdevcmd_failed

cmake %*
exit /b %ERRORLEVEL%

:vswhere_missing
echo ERROR: vswhere not found at "%_VSWHERE%"
exit /b 1

:vs_missing
echo ERROR: Visual Studio with VC tools (x86.x64) not found.
exit /b 1

:vsdevcmd_failed
echo ERROR: VsDevCmd.bat failed.
exit /b 1
