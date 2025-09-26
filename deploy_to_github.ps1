# NaviGrab GitHub Deployment Script
# This script helps deploy the project to the GitHub repository

Write-Host "🚀 NaviGrab GitHub Deployment Script" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""

# Check if git is installed
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "❌ Git is not installed or not in PATH" -ForegroundColor Red
    Write-Host "Please install Git and try again" -ForegroundColor Red
    exit 1
}

# Check if we're in a git repository
if (-not (Test-Path ".git")) {
    Write-Host "🔧 Initializing Git repository..." -ForegroundColor Yellow
    git init
    git remote add origin https://github.com/mcpmessenger/NaviGrab.git
}

# Add all files
Write-Host "📁 Adding files to Git..." -ForegroundColor Yellow
git add .

# Check if there are changes to commit
$status = git status --porcelain
if (-not $status) {
    Write-Host "✅ No changes to commit" -ForegroundColor Green
    exit 0
}

# Commit changes
Write-Host "💾 Committing changes..." -ForegroundColor Yellow
git commit -m "🚀 Initial NaviGrab implementation

✨ Features:
- Real image processing (PNG/JPEG encoding)
- Network layer for HTTP requests
- DOM interaction with Blink engine simulation
- Screenshot capture functionality
- Proactive scraping framework
- Navi Grab web interface
- Comprehensive examples and tests
- CI/CD pipeline setup
- Documentation and deployment guides

🏗️ Architecture:
- Modern C++20 with RAII memory management
- Modular design with clear separation
- Interface-based architecture
- Factory pattern for extensibility
- MCP communication protocol

🧪 Testing:
- Unit tests for all modules
- Integration tests for workflows
- End-to-end tests for user scenarios
- Performance benchmarks

📚 Documentation:
- Comprehensive README
- API reference
- Deployment guides
- Contributing guidelines
- Troubleshooting guide

🚀 Ready for production deployment!"

# Push to GitHub
Write-Host "🌐 Pushing to GitHub..." -ForegroundColor Yellow
git push -u origin main

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "🎉 Successfully deployed to GitHub!" -ForegroundColor Green
    Write-Host "Repository: https://github.com/mcpmessenger/NaviGrab" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "📋 Next steps:" -ForegroundColor Yellow
    Write-Host "1. Check the repository on GitHub" -ForegroundColor White
    Write-Host "2. Review the CI/CD pipeline" -ForegroundColor White
    Write-Host "3. Test the web interface" -ForegroundColor White
    Write-Host "4. Start contributing!" -ForegroundColor White
} else {
    Write-Host "❌ Failed to push to GitHub" -ForegroundColor Red
    Write-Host "Please check your Git configuration and try again" -ForegroundColor Red
}

Write-Host ""
Write-Host "🔗 Useful links:" -ForegroundColor Yellow
Write-Host "- Repository: https://github.com/mcpmessenger/NaviGrab" -ForegroundColor Cyan
Write-Host "- Issues: https://github.com/mcpmessenger/NaviGrab/issues" -ForegroundColor Cyan
Write-Host "- Discussions: https://github.com/mcpmessenger/NaviGrab/discussions" -ForegroundColor Cyan
Write-Host "- Actions: https://github.com/mcpmessenger/NaviGrab/actions" -ForegroundColor Cyan
