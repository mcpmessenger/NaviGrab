# C++ Playwright Clone - Build Script
# This script builds the project using the full CMake path

Write-Host "[INFO] C++ Playwright Clone - Building Project" -ForegroundColor Blue
Write-Host "[INFO] ======================================" -ForegroundColor Blue

# Set CMake path
$cmakePath = "C:\Program Files\CMake\bin\cmake.exe"

# Check if CMake exists
if (-not (Test-Path $cmakePath)) {
    Write-Host "[ERROR] CMake not found at $cmakePath" -ForegroundColor Red
    exit 1
}

# Create build directory
if (-not (Test-Path "build")) {
    Write-Host "[INFO] Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Name "build" | Out-Null
}

# Change to build directory
Set-Location build

# Configure with CMake
Write-Host "[INFO] Configuring project with CMake..." -ForegroundColor Yellow
& $cmakePath .. -DUSE_MOCK_CHROMIUM=ON -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] CMake configuration failed." -ForegroundColor Red
    exit 1
}

# Build the project
Write-Host "[INFO] Building project..." -ForegroundColor Yellow
& $cmakePath --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed." -ForegroundColor Red
    exit 1
}

Write-Host "[SUCCESS] Build completed successfully!" -ForegroundColor Green
Write-Host "[INFO] Build artifacts are in the 'build' directory." -ForegroundColor Cyan
Write-Host "[INFO] You can now run the examples from the build directory." -ForegroundColor Cyan

# Return to parent directory
Set-Location ..
