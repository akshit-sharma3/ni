@echo off
setlocal enabledelayedexpansion

echo ====================================================================
echo   THE ROYAL VAULT - LUXURY PAWN SHOP & COLLATERAL LOANS
echo   C++ Web Server Automated Builder & Runner
echo ====================================================================
echo.

cd /d "%~dp0"

:: 1. Check for MinGW / GCC (g++)
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] GCC / G++ Compiler detected.
    echo [BUILD] Compiling website.c++ with g++ (C++17 + Winsock2)...
    g++ -std=c++17 -O2 "website.c++" -o pawnshop.exe -lws2_32
    if %errorlevel% equ 0 (
        echo [SUCCESS] pawnshop.exe compiled successfully!
        echo [LAUNCH] Starting Royal Vault Web Server...
        start pawnshop.exe
        goto :done
    ) else (
        echo [ERROR] Compilation failed with g++.
        goto :error
    )
)

:: 2. Check for Clang++
where clang++ >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] Clang++ Compiler detected.
    echo [BUILD] Compiling website.c++ with clang++...
    clang++ -std=c++17 -O2 "website.c++" -o pawnshop.exe -lws2_32
    if %errorlevel% equ 0 (
        echo [SUCCESS] pawnshop.exe compiled successfully!
        echo [LAUNCH] Starting Royal Vault Web Server...
        start pawnshop.exe
        goto :done
    ) else (
        echo [ERROR] Compilation failed with clang++.
        goto :error
    )
)

:: 3. Check for MSVC (cl.exe)
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] Microsoft Visual C++ Compiler (cl.exe) detected.
    echo [BUILD] Compiling website.c++ with MSVC...
    cl /EHsc /std:c++17 /O2 "website.c++" /Fe:pawnshop.exe ws2_32.lib
    if %errorlevel% equ 0 (
        echo [SUCCESS] pawnshop.exe compiled successfully!
        echo [LAUNCH] Starting Royal Vault Web Server...
        start pawnshop.exe
        goto :done
    ) else (
        echo [ERROR] Compilation failed with cl.
        goto :error
    )
)

echo [NOTICE] No C++ compiler (g++, clang++, or cl.exe) found in standard PATH.
echo.
echo To compile and run this C++ pawn shop website:
echo   1. Install MinGW-w64 (via MSYS2 or WinLibs) or Visual Studio Build Tools.
echo   2. Run:
echo        g++ -std=c++17 website.c++ -o pawnshop.exe -lws2_32
echo      or
echo        cl /EHsc /std:c++17 website.c++ ws2_32.lib
echo   3. Run: pawnshop.exe
echo.
echo Note: If you want to preview the website immediately without compiling C++,
echo an HTML standalone preview is also available as "preview.html".
pause
exit /b 1

:done
echo.
echo The server is running and your browser has been opened to http://localhost:8080
pause
exit /b 0

:error
echo.
echo An error occurred during compilation.
pause
exit /b 1
