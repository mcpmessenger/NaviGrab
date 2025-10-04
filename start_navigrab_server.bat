@echo off
echo 🕷️ Starting NaviGrab Screenshot Server...
echo =============================================
echo.
echo This will:
echo 1. Install Node.js dependencies (if needed)
echo 2. Start the screenshot server
echo 3. Open the web interface
echo.

echo 📦 Checking Node.js installation...
node --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Node.js not found. Please install Node.js first.
    echo Download from: https://nodejs.org/
    pause
    exit /b 1
)

echo ✅ Node.js found: 
node --version

echo.
echo 📦 Installing dependencies...
call npm install express puppeteer

echo.
echo 🚀 Starting NaviGrab Screenshot Server...
echo Server will run on http://localhost:8080
echo.

start "" "http://localhost:8080"

echo 🌐 Opening web interface...
echo ✅ NaviGrab Screenshot Server is starting!
echo.
echo 📋 Features:
echo    ✅ Cross-origin screenshot capture
echo    ✅ Link extraction and analysis
echo    ✅ Page statistics
echo    ✅ OpenAI integration ready
echo.
echo 🎯 The web interface should open automatically
echo Press Ctrl+C to stop the server
echo.

node screenshot_server.js
