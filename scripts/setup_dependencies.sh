#!/bin/bash

# C++ Playwright Clone - Dependency Setup Script
# This script sets up all required dependencies for building the project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Function to install dependencies on Linux
install_linux_deps() {
    print_status "Installing dependencies on Linux..."
    
    # Update package list
    sudo apt-get update
    
    # Install build essentials
    sudo apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        git \
        curl \
        wget
    
    # Install C++ compiler
    if ! command_exists g++-10; then
        sudo apt-get install -y g++-10
        sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100
    fi
    
    # Install development libraries
    sudo apt-get install -y \
        libsqlite3-dev \
        libssl-dev \
        libcurl4-openssl-dev \
        libpng-dev \
        libjpeg-dev \
        libwebp-dev \
        zlib1g-dev \
        libbz2-dev \
        liblzma-dev
    
    # Install Chromium dependencies (if available)
    if command_exists apt-cache && apt-cache search libnss3-dev >/dev/null 2>&1; then
        sudo apt-get install -y \
            libnss3-dev \
            libatk-bridge2.0-dev \
            libdrm2 \
            libxcomposite1 \
            libxdamage1 \
            libxrandr2 \
            libgbm1 \
            libxss1 \
            libasound2
    else
        print_warning "Chromium dependencies not available in this repository"
    fi
    
    print_success "Linux dependencies installed successfully"
}

# Function to install dependencies on macOS
install_macos_deps() {
    print_status "Installing dependencies on macOS..."
    
    # Check if Homebrew is installed
    if ! command_exists brew; then
        print_status "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Install build tools
    brew install cmake ninja pkg-config
    
    # Install C++ compiler
    brew install gcc@11
    
    # Install development libraries
    brew install sqlite openssl curl libpng libjpeg webp zlib bzip2 xz
    
    # Install Chromium dependencies
    brew install nss atk-bridge libdrm libxcomposite libxdamage libxrandr libgbm libxss alsa-lib
    
    print_success "macOS dependencies installed successfully"
}

# Function to install dependencies on Windows
install_windows_deps() {
    print_status "Installing dependencies on Windows..."
    
    # Check if vcpkg is installed
    if [ ! -d "vcpkg" ]; then
        print_status "Installing vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git
        cd vcpkg
        ./bootstrap-vcpkg.bat
        cd ..
    fi
    
    # Install packages via vcpkg
    ./vcpkg/vcpkg install sqlite3 openssl curl libpng libjpeg-turbo libwebp zlib bzip2 lzma
    
    # Install Visual Studio Build Tools (if not already installed)
    if ! command_exists cl; then
        print_warning "Visual Studio Build Tools not found. Please install Visual Studio 2019 or later with C++ development tools."
        print_status "You can download it from: https://visualstudio.microsoft.com/downloads/"
    fi
    
    print_success "Windows dependencies installed successfully"
}

# Function to setup Chromium (optional)
setup_chromium() {
    print_status "Setting up Chromium integration..."
    
    if [ -z "$CHROMIUM_ROOT_DIR" ]; then
        print_warning "CHROMIUM_ROOT_DIR not set. Skipping Chromium setup."
        print_status "To enable full Chromium integration, set CHROMIUM_ROOT_DIR to your Chromium source directory."
        return
    fi
    
    if [ ! -d "$CHROMIUM_ROOT_DIR" ]; then
        print_error "Chromium directory not found: $CHROMIUM_ROOT_DIR"
        return
    fi
    
    print_status "Chromium root directory: $CHROMIUM_ROOT_DIR"
    print_success "Chromium integration ready"
}

# Function to create build directory
setup_build() {
    print_status "Setting up build directory..."
    
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    # Configure CMake
    print_status "Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_CXX_STANDARD=20 \
             -DCMAKE_CXX_STANDARD_REQUIRED=ON \
             -DCMAKE_CXX_EXTENSIONS=OFF
    
    print_success "Build directory configured"
}

# Function to build the project
build_project() {
    print_status "Building the project..."
    
    cd build
    
    # Build with all available cores
    cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    print_success "Project built successfully"
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    cd build
    
    # Run CTest
    ctest --output-on-failure
    
    print_success "All tests passed"
}

# Function to install the project
install_project() {
    print_status "Installing the project..."
    
    cd build
    
    # Install to system (optional)
    if [ "$1" = "--install" ]; then
        sudo cmake --install .
        print_success "Project installed to system"
    else
        print_status "To install to system, run: $0 --install"
    fi
}

# Function to clean build
clean_build() {
    print_status "Cleaning build directory..."
    
    if [ -d "build" ]; then
        rm -rf build
        print_success "Build directory cleaned"
    else
        print_warning "No build directory found"
    fi
}

# Function to show help
show_help() {
    echo "C++ Playwright Clone - Dependency Setup Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --help, -h          Show this help message"
    echo "  --install           Install the project to system after building"
    echo "  --clean             Clean build directory before setup"
    echo "  --no-build          Skip building the project"
    echo "  --no-tests          Skip running tests"
    echo "  --chromium-dir DIR  Set Chromium root directory"
    echo ""
    echo "Examples:"
    echo "  $0                                    # Basic setup and build"
    echo "  $0 --install                         # Setup, build, and install"
    echo "  $0 --clean --install                 # Clean, setup, build, and install"
    echo "  $0 --chromium-dir /path/to/chromium  # Setup with Chromium integration"
}

# Main function
main() {
    print_status "C++ Playwright Clone - Dependency Setup"
    print_status "========================================"
    
    # Parse command line arguments
    CLEAN_BUILD=false
    NO_BUILD=false
    NO_TESTS=false
    INSTALL_PROJECT=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help|-h)
                show_help
                exit 0
                ;;
            --install)
                INSTALL_PROJECT=true
                shift
                ;;
            --clean)
                CLEAN_BUILD=true
                shift
                ;;
            --no-build)
                NO_BUILD=true
                shift
                ;;
            --no-tests)
                NO_TESTS=true
                shift
                ;;
            --chromium-dir)
                export CHROMIUM_ROOT_DIR="$2"
                shift 2
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Detect operating system
    OS=$(detect_os)
    print_status "Detected OS: $OS"
    
    # Clean build if requested
    if [ "$CLEAN_BUILD" = true ]; then
        clean_build
    fi
    
    # Install dependencies based on OS
    case $OS in
        linux)
            install_linux_deps
            ;;
        macos)
            install_macos_deps
            ;;
        windows)
            install_windows_deps
            ;;
        *)
            print_error "Unsupported operating system: $OS"
            exit 1
            ;;
    esac
    
    # Setup Chromium integration
    setup_chromium
    
    # Setup build directory
    setup_build
    
    # Build project if not skipped
    if [ "$NO_BUILD" = false ]; then
        build_project
        
        # Run tests if not skipped
        if [ "$NO_TESTS" = false ]; then
            run_tests
        fi
        
        # Install project if requested
        if [ "$INSTALL_PROJECT" = true ]; then
            install_project --install
        fi
    fi
    
    print_success "Setup completed successfully!"
    print_status "You can now use the C++ Playwright Clone library in your projects."
    
    if [ "$NO_BUILD" = false ]; then
        print_status "Build artifacts are available in the 'build' directory."
        print_status "To run examples: ./build/bin/basic_usage"
    fi
}

# Run main function with all arguments
main "$@"
