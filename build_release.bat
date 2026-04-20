@echo off
setlocal

echo ========================================
echo Building LTCSurveyReadinessApp
echo ========================================

if exist build (
    echo Removing old build directory...
    rmdir /s /q build
)

cmake -S . -B build -G "Visual Studio 17 2022"
if errorlevel 1 goto :fail

cmake --build build --config Release
if errorlevel 1 goto :fail

echo.
echo Build complete.
echo Run:
echo build\Release\LTCSurveyReadinessApp.exe
goto :eof

:fail
echo.
echo Build failed.
exit /b 1
