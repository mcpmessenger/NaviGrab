# Simple GitHub Deployment Script
Write-Host "🚀 Deploying NaviGrab to GitHub..." -ForegroundColor Green

# Initialize git if needed
if (-not (Test-Path ".git")) {
    Write-Host "🔧 Initializing Git repository..." -ForegroundColor Yellow
    git init
    git remote add origin https://github.com/mcpmessenger/NaviGrab.git
}

# Add all files
Write-Host "📁 Adding files..." -ForegroundColor Yellow
git add .

# Commit
Write-Host "💾 Committing..." -ForegroundColor Yellow
git commit -m "🚀 Initial NaviGrab implementation with all features"

# Push
Write-Host "🌐 Pushing to GitHub..." -ForegroundColor Yellow
git push -u origin main

Write-Host "✅ Deployment complete!" -ForegroundColor Green
