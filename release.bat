@echo off
setlocal

echo "%~nx0 re" to rebuild x64-release and "%~nx0" to simple build

set PRESET=x64-release
set BUILD_DIR=out\build\%PRESET%

if "%1"=="re" rmdir /s /q "%BUILD_DIR%"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1

if not exist glog\ (
    7z x glog.7z
    if errorlevel 1 exit /b 1
)

cmake --preset=%PRESET%
if errorlevel 1 exit /b 1

cmake --build %BUILD_DIR%
if errorlevel 1 exit /b 1

if exist glog\bin\glog.dll (
    if not exist %BUILD_DIR%\glog.dll copy glog\bin\glog.dll %BUILD_DIR%\glog.dll >nul
)

if exist inlet.in (
    if not exist %BUILD_DIR%\inlet.in copy inlet.in %BUILD_DIR%\inlet.in >nul
)

endlocal