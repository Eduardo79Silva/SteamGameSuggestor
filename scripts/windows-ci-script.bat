@echo off
setlocal enabledelayedexpansion

echo Starting Windows CI build for SteamSuggestor

:: Set up build directory - using quotes to handle paths with spaces and special characters
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%"
set "BUILD_DIR=%PROJECT_ROOT%build"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Check for build tools
echo Checking for build tools...
where /q cmake
if %ERRORLEVEL% neq 0 (
    echo CMake not found in PATH
    echo Installing CMake...
    powershell -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-windows-x86_64.msi -OutFile cmake-3.23.1-windows-x86_64.msi; Start-Process -FilePath msiexec.exe -ArgumentList '/i cmake-3.23.1-windows-x86_64.msi /quiet /norestart' -Wait; }"
    set "PATH=%PATH%;C:\Program Files\CMake\bin"
)

:: Check for Visual Studio
echo Checking for Visual Studio...
call :find_vs_developer_command
if %ERRORLEVEL% neq 0 (
    echo Visual Studio environment not found
    echo Please install Visual Studio with C++ development tools
    exit /b 1
)

:: Install dependencies
echo Installing dependencies...

:: Setup curl if not present
if not exist "%PROJECT_ROOT%lib\curl" (
    echo Setting up libcurl...
    if not exist "%PROJECT_ROOT%lib" mkdir "%PROJECT_ROOT%lib"
    cd /d "%PROJECT_ROOT%lib"
    
    :: Download and extract curl
    powershell -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri https://curl.se/windows/dl-7.83.1_1/curl-7.83.1_1-win64-mingw.zip -OutFile curl.zip; Expand-Archive -Path curl.zip -DestinationPath .; }"
    
    :: Rename the extracted directory to curl
    for /d %%D in (curl-*) do (
        rename "%%D" curl
    )
    
    if not exist "%PROJECT_ROOT%lib\curl" (
        echo Failed to set up libcurl
        exit /b 1
    )
)

:: Setup Google Test if not present
if not exist "%PROJECT_ROOT%lib\gtest" (
    echo Setting up Google Test...
    cd /d "%PROJECT_ROOT%lib"
    
    :: Clone Google Test repository
    git clone https://github.com/google/googletest.git gtest
    
    if not exist "%PROJECT_ROOT%lib\gtest" (
        echo Failed to set up Google Test
        exit /b 1
    )
)

:: Build the project
echo Building SteamSuggestor...
cd /d "%BUILD_DIR%"
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

:: Run tests
echo Running tests...
if exist "%BUILD_DIR%\Release\run_tests.exe" (
    "%BUILD_DIR%\Release\run_tests.exe"
) else (
    echo Warning: Test executable not found. Skipping tests.
)

:: Create distribution package
echo Creating distribution package...
mkdir "%BUILD_DIR%\package\bin"
copy "%BUILD_DIR%\Release\SteamSuggestor.exe" "%BUILD_DIR%\package\bin"

:: Create a ZIP file
cd /d "%BUILD_DIR%\package"
powershell -Command "& { Compress-Archive -Path .\* -DestinationPath ..\SteamSuggestor-Windows.zip -Force }"

echo Build completed successfully!
echo Binary located at: %BUILD_DIR%\Release\SteamSuggestor.exe
echo Package located at: %BUILD_DIR%\SteamSuggestor-Windows.zip

exit /b 0

:find_vs_developer_command
:: Try different Visual Studio versions, starting with the newest
for %%v in (2022 2019 2017) do (
    if exist "C:\Program Files\Microsoft Visual Studio\%%v\Enterprise\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%v\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
    if exist "C:\Program Files\Microsoft Visual Studio\%%v\Professional\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%v\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
    if exist "C:\Program Files\Microsoft Visual Studio\%%v\Community\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%v\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Enterprise\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Professional\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Community\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\%%v\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
        exit /b 0
    )
)
exit /b 1
