# C++ Playwright Clone - Dependency Setup Script (Windows PowerShell)
# This script sets up all required dependencies for building the project on Windows

param(
    [switch]$Install,
    [switch]$Clean,
    [switch]$NoBuild,
    [switch]$NoTests,
    [string]$ChromiumDir = ""
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Colors for output
$Red = "Red"
$Green = "Green"
$Yellow = "Yellow"
$Blue = "Blue"

# Function to print colored output
function Write-Status {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor $Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor $Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor $Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor $Red
}

# Function to check if command exists
function Test-Command {
    param([string]$Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    }
    catch {
        return $false
    }
}

# Function to install dependencies
function Install-Dependencies {
    Write-Status "Installing dependencies on Windows..."
    
    # Check if Chocolatey is installed
    if (-not (Test-Command "choco")) {
        Write-Status "Installing Chocolatey package manager..."
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    }
    
    # Install build tools
    Write-Status "Installing build tools..."
    choco install -y cmake ninja git
    
    # Install Visual Studio Build Tools
    if (-not (Test-Command "cl")) {
        Write-Status "Installing Visual Studio Build Tools..."
        choco install -y visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools"
    }
    
    # Install development libraries
    Write-Status "Installing development libraries..."
    choco install -y sqlite openssl curl
    
    # Install vcpkg for C++ packages
    if (-not (Test-Directory "vcpkg")) {
        Write-Status "Installing vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git
        Set-Location vcpkg
        .\bootstrap-vcpkg.bat
        Set-Location ..
    }
    
    # Install packages via vcpkg
    Write-Status "Installing C++ packages via vcpkg..."
    .\vcpkg\vcpkg install sqlite3 openssl curl libpng libjpeg-turbo libwebp zlib bzip2 lzma
    
    Write-Success "Dependencies installed successfully"
}

# Function to setup Chromium
function Setup-Chromium {
    if ($ChromiumDir) {
        Write-Status "Setting up Chromium integration..."
        $env:CHROMIUM_ROOT_DIR = $ChromiumDir
        
        if (-not (Test-Path $ChromiumDir)) {
            Write-Error "Chromium directory not found: $ChromiumDir"
            return
        }
        
        Write-Status "Chromium root directory: $ChromiumDir"
        Write-Success "Chromium integration ready"
    } else {
        Write-Warning "CHROMIUM_ROOT_DIR not set. Skipping Chromium setup."
        Write-Status "To enable full Chromium integration, use -ChromiumDir parameter."
    }
}

# Function to create build directory
function Setup-Build {
    Write-Status "Setting up build directory..."
    
    if (-not (Test-Path "build")) {
        New-Item -ItemType Directory -Name "build" | Out-Null
    }
    
    Set-Location build
    
    # Configure CMake
    Write-Status "Configuring CMake..."
    $cmakeArgs = @(
        "..",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_STANDARD=20",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
        "-DCMAKE_CXX_EXTENSIONS=OFF"
    )
    
    if ($ChromiumDir) {
        $cmakeArgs += "-DCHROMIUM_ROOT_DIR=$ChromiumDir"
    } else {
        $cmakeArgs += "-DUSE_MOCK_CHROMIUM=ON"
    }
    
    & cmake @cmakeArgs
    
    Write-Success "Build directory configured"
}

# Function to build the project
function Build-Project {
    Write-Status "Building the project..."
    
    Set-Location build
    
    # Build with all available cores
    $cores = [Environment]::ProcessorCount
    & cmake --build . --parallel $cores
    
    Write-Success "Project built successfully"
}

# Function to run tests
function Run-Tests {
    Write-Status "Running tests..."
    
    Set-Location build
    
    # Run CTest
    & ctest --output-on-failure
    
    Write-Success "All tests passed"
}

# Function to install the project
function Install-Project {
    Write-Status "Installing the project..."
    
    Set-Location build
    
    if ($Install) {
        & cmake --install .
        Write-Success "Project installed to system"
    } else {
        Write-Status "To install to system, run: .\scripts\setup_dependencies.ps1 -Install"
    }
}

# Function to clean build
function Clean-Build {
    Write-Status "Cleaning build directory..."
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
        Write-Success "Build directory cleaned"
    } else {
        Write-Warning "No build directory found"
    }
}

# Function to show help
function Show-Help {
    Write-Host "C++ Playwright Clone - Dependency Setup Script (Windows)" -ForegroundColor $Blue
    Write-Host ""
    Write-Host "Usage: .\scripts\setup_dependencies.ps1 [OPTIONS]" -ForegroundColor $Yellow
    Write-Host ""
    Write-Host "Options:" -ForegroundColor $Yellow
    Write-Host "  -Install           Install the project to system after building"
    Write-Host "  -Clean             Clean build directory before setup"
    Write-Host "  -NoBuild           Skip building the project"
    Write-Host "  -NoTests           Skip running tests"
    Write-Host "  -ChromiumDir DIR   Set Chromium root directory"
    Write-Host "  -Help              Show this help message"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor $Yellow
    Write-Host "  .\scripts\setup_dependencies.ps1                                    # Basic setup and build"
    Write-Host "  .\scripts\setup_dependencies.ps1 -Install                          # Setup, build, and install"
    Write-Host "  .\scripts\setup_dependencies.ps1 -Clean -Install                   # Clean, setup, build, and install"
    Write-Host "  .\scripts\setup_dependencies.ps1 -ChromiumDir C:\chromium\src      # Setup with Chromium integration"
}

# Main function
function Main {
    Write-Status "C++ Playwright Clone - Dependency Setup (Windows)"
    Write-Status "================================================"
    
    # Show help if requested
    if ($args -contains "-Help" -or $args -contains "-h") {
        Show-Help
        return
    }
    
    try {
        # Clean build if requested
        if ($Clean) {
            Clean-Build
        }
        
        # Install dependencies
        Install-Dependencies
        
        # Setup Chromium integration
        Setup-Chromium
        
        # Setup build directory
        Setup-Build
        
        # Build project if not skipped
        if (-not $NoBuild) {
            Build-Project
            
            # Run tests if not skipped
            if (-not $NoTests) {
                Run-Tests
            }
            
            # Install project if requested
            Install-Project
        }
        
        Write-Success "Setup completed successfully!"
        Write-Status "You can now use the C++ Playwright Clone library in your projects."
        
        if (-not $NoBuild) {
            Write-Status "Build artifacts are available in the 'build' directory."
            Write-Status "To run examples: .\build\bin\basic_usage.exe"
        }
        
    } catch {
        Write-Error "Setup failed: $($_.Exception.Message)"
        exit 1
    }
}

# Run main function
Main
