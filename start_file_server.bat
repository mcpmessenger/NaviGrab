@echo off
echo 🌐 Starting NaviGrab File Server
echo ================================

echo 📁 Current directory: %CD%
echo 🔗 Starting Python HTTP server on port 8080...
echo.
echo ✅ Server will serve scraped data files
echo 🌐 Open your browser and go to:
echo    http://localhost:8080/web_interface/proactive_scraper.html
echo.
echo Press Ctrl+C to stop the server
echo.

python -m http.server 8080


