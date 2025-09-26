@echo off
REM C++ Playwright Clone - Simple Setup Script (Windows)
REM This script sets up the project without requiring admin rights

echo [INFO] C++ Playwright Clone - Simple Setup
echo [INFO] ====================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo [ERROR] CMakeLists.txt not found. Please run this script from the project root directory.
    pause
    exit /b 1
)

REM Create build directory
echo [INFO] Creating build directory...
if not exist "build" mkdir build
cd build

REM Check for CMake
echo [INFO] Checking for CMake...
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] CMake not found. Please install CMake first.
    echo [INFO] You can download CMake from: https://cmake.org/download/
    echo [INFO] Or install via winget: winget install Kitware.CMake
    echo [INFO] Or install via chocolatey: choco install cmake (requires admin)
    pause
    exit /b 1
)

REM Configure with CMake
echo [INFO] Configuring project with CMake...
cmake .. -DUSE_MOCK_CHROMIUM=ON -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)

REM Build the project
echo [INFO] Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo [SUCCESS] Setup completed successfully!
echo [INFO] Build artifacts are in the 'build' directory.
echo [INFO] You can now run the examples from the build directory.

pause
