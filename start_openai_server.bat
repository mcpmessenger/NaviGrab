@echo off
echo 🤖 Starting NaviGrab OpenAI Server...
echo ====================================
echo.
echo This server provides screenshot analysis using OpenAI Vision API
echo Server will run on localhost:8080
echo.
echo Prerequisites:
echo - OpenAI API key (set via web interface)
echo - Internet connection for API calls
echo.
echo Press Ctrl+C to stop the server
echo.

cd build\bin\Release
.\navigrab_openai_server.exe

pause

