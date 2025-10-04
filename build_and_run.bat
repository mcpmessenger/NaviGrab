@echo off
echo 🚀 Building NaviGrab Web Server...
echo =================================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo 📋 Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Build the project
echo 🔨 Building project...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Build failed!
    pause
    exit /b 1
)

echo ✅ Build successful!
echo.

REM Start the web server
echo 🌐 Starting Web Server...
echo 📡 Server will be available at: http://localhost:8080
echo 🛑 Press Ctrl+C to stop the server
echo.

cd bin
web_server.exe

pause

