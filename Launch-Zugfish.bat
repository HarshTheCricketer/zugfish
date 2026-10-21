```bat
@echo off
title Zugfish Chess Review

set "PROJECT=C:\Users\hvs14\Desktop\Personal Coding\Zugfish Chess Review App\zugfish-enhanced\zugfish"
set "MSYS2=C:\msys64"

if not exist "%PROJECT%\build\gui\zugfish-qt.exe" (
    echo ERROR: Zugfish executable not found.
    echo Expected:
    echo %PROJECT%\build\gui\zugfish-qt.exe
    pause
    exit /b 1
)

if not exist "%MSYS2%\usr\bin\bash.exe" (
    echo ERROR: MSYS2 Bash not found at %MSYS2%\usr\bin\bash.exe
    pause
    exit /b 1
)

set "MSYSTEM=UCRT64"
set "CHERE_INVOKING=1"
set "PATH=%MSYS2%\ucrt64\bin;%MSYS2%\usr\bin;%PATH%"

"%MSYS2%\usr\bin\bash.exe" --login -c "cd '/c/Users/hvs14/Desktop/Personal Coding/Zugfish Chess Review App/zugfish-enhanced/zugfish' && ./build/gui/zugfish-qt.exe"

if errorlevel 1 (
    echo.
    echo Zugfish exited with an error.
    pause
)
```