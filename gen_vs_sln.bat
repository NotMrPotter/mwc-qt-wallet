@echo off
rem *****************************************
rem Generates a fresh VS solution using CMake
rem *****************************************
echo.

rem Clean env
set _ARCH=
set CMAKE_BIN=
set _QT5_DIR=
set QT5_DIR=
set STATIC=
set _TARCH=
set _VSVER=
set _YEAR=

rem Try to locate cmake
where cmake 1>NUL 2>NUL
if %ERRORLEVEL% equ 0 (
	for /f "tokens=*" %%i in ('where cmake') do set CMAKE_BIN=%%i
) else (
	set CMAKE_BIN=C:\Program Files\CMake\bin\cmake.exe
)
if not defined CMAKE_BIN goto :cmerr

echo Using CMake: %CMAKE_BIN%

rem Visual Studio version as set by vcvarsXXX
set _VSVER=%VisualStudioVersion%
if not defined _VSVER goto :vserr

rem Try to pick an appropriate target arch
if "%1"=="/32" set _TARCH=x86
if "%2"=="/32" set _TARCH=x86
if "%3"=="/32" set _TARCH=x86
if "%4"=="/32" set _TARCH=x86
if "%1"=="/static" set STATIC=-DStatic_Build:BOOL=true
if "%2"=="/static" set STATIC=-DStatic_Build:BOOL=true
if "%3"=="/static" set STATIC=-DStatic_Build:BOOL=true
if "%4"=="/static" set STATIC=-DStatic_Build:BOOL=true
if "%1"=="/qt5" set QT5_DIR=-DQt5_DIR:PATH="%2"
if "%2"=="/qt5" set QT5_DIR=-DQt5_DIR:PATH="%3"
if "%3"=="/qt5" set QT5_DIR=-DQt5_DIR:PATH="%4"
if not defined _TARCH set _TARCH=%VSCMD_ARG_TGT_ARCH%
if not defined _TARCH set _TARCH=x64

echo Using Visual Studio: %_VSVER%, target architecture: %_TARCH%
if not defined VSCMD_ARG_TGT_ARCH (
	if %_TARCH%==x64 (
		echo.
		echo ====
		echo WARN: Desired architecture cannot be detected on VS2015 and below.
		echo Defaulting to 64-bit. Please re-run this script with the /32 flag
		echo if you need to build for 32-bit.
		echo ====
		echo.
		pause
		echo.
	)
)

rem Create or clean target directory
mkdir VS\%_TARCH% 2>NUL
cd VS\%_TARCH%
del /f/q/s CMakeFiles 2>1>NUL
del CMakeCache.txt 2>1>NUL



rem Check provided Qt5_DIR before proceeding
if not defined QT5_DIR (
	echo.
	echo ====
	echo STOP: Using default Qt 5.9.8 install directory. If Qt is installed somewhere
	echo else, specify it like /qt=C:\path\to\qt\5.9.8\msvc2015\lib\cmake\qt5
	echo ====
	echo.
	pause
	echo.
) else (
	echo.
	echo Using Qt at %QT5_DIR%
	echo.
)



rem Run CMake with the appropriate VS generator
if "%_VSVER%"=="14.0" goto :vs14
if "%_VSVER%"=="15.0" goto :vs15
if "%_VSVER%"=="16.0" goto :vs16

:vs14
set _YEAR=2015
if "%_TARCH%"=="x64" set _ARCH= Win64
"%CMAKE_BIN%" -G"Visual Studio 14 2015%_ARCH%" %STATIC% %QT5_DIR% ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success

:vs15
set _YEAR=2017
if "%_TARCH%"=="x64" set _ARCH= Win64
"%CMAKE_BIN%" -G"Visual Studio 15 2017%_ARCH%" %STATIC% %QT5_DIR% ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success

:vs16
set _YEAR=2019
if "%_TARCH%"=="x64" set _ARCH= x64
"%CMAKE_BIN%" -G"Visual Studio 16 2019" -A%_ARCH% %STATIC% %QT5_DIR% ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success



:success
echo.
echo ====
echo SUCCESS: You can find the generated Visual Studio %_YEAR% solution at
echo %CD%\mwc-qt-wallet.sln
echo ====
cd ..\..
goto :end



:cmerr
echo.
echo ====
echo FATAL: Could not locate CMake. Please add it to your path or edit the CMAKE_BIN
echo variable at the beginning of this script.
echo ====
goto :end

:vserr
echo.
echo ====
echo FATAL: Could not detect Visual Studio version. Please run this script from
echo a Visual Studio developer command prompt or call vcvarsall.bat manually.
echo ====
goto :fail

:generr
echo.
echo ===
echo FATAL: CMake could not generate the Visual Studio %_YEAR% solution. Please check
echo the output above and ensure you have installed all the prerequisites outlined in
for %%a in ("%CD%\..\..") DO echo %%~dpaDOCS\BUILD_WINDOWS.md
echo ===
goto :fail



:fail
rem del /f/q/s *.* >NUL
cd ..
rem rmdir /Q/S %_TARCH%
cd ..

:end
rem Clean env
set CMAKE_BIN=
set _VSVER=
set _TARCH=
set _YEAR=
echo.
pause
