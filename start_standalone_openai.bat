@echo off
echo 🤖 Starting NaviGrab Standalone OpenAI Demo...
echo =============================================
echo.
echo This will:
echo 1. Build the C++ OpenAI screenshot demo
echo 2. Open the standalone web interface
echo 3. Show you how to use OpenAI analysis
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

REM Open the standalone web interface
echo 🌐 Opening standalone web interface...
start "" "web_interface\standalone_openai_analyzer.html"

echo.
echo 🎉 Setup complete!
echo.
echo 📋 What's Available:
echo    ✅ C++ OpenAI demo (built and ready)
echo    ✅ Standalone web interface (opened in browser)
echo    ✅ Your API key is pre-configured
echo.
echo 📋 Instructions:
echo 1. The web interface is now open in your browser
echo 2. Click "Run C++ Demo" for real OpenAI analysis
echo 3. Click "Simulate Analysis" to see example results
echo 4. The C++ demo will capture your screen and analyze it
echo.
echo 💡 C++ Demo Location:
echo    build\bin\Release\real_openai_screenshot_demo.exe
echo.
echo 🔑 API Key: Already configured in C++ demo
echo.
echo 🛑 Press any key to exit...
pause > nul

