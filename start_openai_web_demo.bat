@echo off
echo 🤖 Starting NaviGrab OpenAI Web Demo...
echo =====================================
echo.
echo This will:
echo 1. Build the OpenAI screenshot demo
echo 2. Open the web interface
echo 3. Show you how to use OpenAI analysis
echo.
echo Make sure you have your OpenAI API key ready!
echo.

REM Build the demo
echo 📦 Building OpenAI demo...
cd build
cmake --build . --config Release --target real_openai_screenshot_demo
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Build failed!
    pause
    exit /b 1
)

echo ✅ Build successful!
echo.

REM Open the web interface
echo 🌐 Opening web interface...
start "" "web_interface\openai_screenshot_analyzer.html"

echo.
echo 🎉 Setup complete!
echo.
echo 📋 Instructions:
echo 1. The web interface should now be open in your browser
echo 2. Enter your OpenAI API key in the configuration section
echo 3. Enter a URL to analyze (e.g., https://github.com)
echo 4. Click "Capture & Analyze" to see OpenAI analysis results
echo.
echo 💡 The C++ demo executable is ready at:
echo    build\bin\Release\real_openai_screenshot_demo.exe
echo.
echo 🛑 Press any key to exit...
pause > nul

