@echo off
echo 🤖 Starting NaviGrab Simple OpenAI Demo...
echo =========================================
echo.
echo This will:
echo 1. Build the C++ OpenAI screenshot demo
echo 2. Open the web interface directly
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

REM Open the web interface directly
echo 🌐 Opening web interface...
start "" "web_interface\openai_screenshot_analyzer.html"

echo.
echo 🎉 Setup complete!
echo.
echo 📋 Instructions:
echo 1. The web interface should now be open in your browser
echo 2. The web interface will simulate OpenAI analysis results
echo 3. For real analysis, run the C++ demo directly:
echo.
echo 💡 Run real OpenAI analysis:
echo    build\bin\Release\real_openai_screenshot_demo.exe
echo.
echo 🔑 Your API key is already configured in the C++ demo
echo.
echo 🛑 Press any key to exit...
pause > nul

