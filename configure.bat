@echo off
REM x64dbg-mcp CMake Configuration Script
REM Usage: configure.bat [SDK_PATH]

setlocal

REM Check if SDK path is provided as first argument
set "XDBG_ARCH=x64"
if "%~1" NEQ "" (
    set XDBG_SDK=%~1
) else (
    REM Prompt user for SDK path
    set /p XDBG_SDK="Enter xdbg SDK path (e.g., C:\x64dbg\pluginsdk or C:\x32dbg\pluginsdk): "
)

REM Check optional arch argument
if "%~2" NEQ "" (
    if /i "%~2"=="x86" set "XDBG_ARCH=x86"
    if /i "%~2"=="x64" set "XDBG_ARCH=x64"
)

REM Set vcpkg path (use environment variable or prompt)
if not defined VCPKG_ROOT (
    set /p VCPKG_ROOT="Enter vcpkg root path (e.g., C:\vcpkg): "
)

REM Verify SDK path exists
if not exist "%X64DBG_SDK%" (
    echo ERROR: SDK path does not exist: %X64DBG_SDK%
    echo Please check the path and try again.
    exit /b 1
)

REM Clean old build directory
if exist build (
    echo Cleaning old build directory...
    rmdir /s /q build
)

REM Run CMake configuration
echo Configuring project...
echo SDK Path: %XDBG_SDK%
echo Target Arch: %XDBG_ARCH%
echo vcpkg: %VCPKG_ROOT%
echo.

set "VCPKG_TRIPLET=x64-windows"
if "%XDBG_ARCH%"=="x86" set "VCPKG_TRIPLET=x86-windows"

cmake -B build ^
    -DXDBG_SDK_DIR="%XDBG_SDK%" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DVCPKG_TARGET_TRIPLET=%VCPKG_TRIPLET% ^
    -DXDBG_ARCH=%XDBG_ARCH%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Configuration failed!
    exit /b 1
)

echo.
echo Configuration successful!
echo.
echo Next steps:
echo   1. Build: cmake --build build --config Release
echo   2. Or open: build\x64dbg_mcp.sln in Visual Studio
echo.

endlocal
