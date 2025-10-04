@echo off
echo 🤖 Starting NaviGrab Complete OpenAI Demo...
echo ==========================================
echo.
echo This will:
echo 1. Build the C++ OpenAI screenshot demo
echo 2. Start the Python web server
echo 3. Open the web interface in your browser
echo 4. Show you how to use OpenAI analysis
echo.
echo Make sure you have your OpenAI API key ready!
echo.

REM Build the C++ demo
echo 📦 Building C++ OpenAI demo...
cd build
cmake --build . --config Release --target real_openai_screenshot_demo
if %ERRORLEVEL% NEQ 0 (
    echo ❌ C++ build failed!
    pause
    exit /b 1
)

echo ✅ C++ build successful!
echo.

REM Go back to project root
cd ..

REM Start the Python web server in background
echo 🌐 Starting Python web server...
start /B python openai_web_server.py

REM Wait a moment for server to start
timeout /t 3 /nobreak > nul

REM Open the web interface
echo 🔗 Opening web interface...
start "" "http://localhost:8080"

echo.
echo 🎉 Complete setup finished!
echo.
echo 📋 What's running:
echo    ✅ C++ OpenAI demo (built and ready)
echo    ✅ Python web server (http://localhost:8080)
echo    ✅ Web interface (opened in browser)
echo.
echo 📋 Instructions:
echo 1. The web interface should now be open in your browser
echo 2. Enter your OpenAI API key in the configuration section
echo 3. Enter a URL to analyze (e.g., https://github.com)
echo 4. Click "Capture & Analyze" to see OpenAI analysis results
echo.
echo 💡 You can also run the C++ demo directly:
echo    build\bin\Release\real_openai_screenshot_demo.exe
echo.
echo 🛑 Press any key to exit (server will keep running)...
pause > nul

