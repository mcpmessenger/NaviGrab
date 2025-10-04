#!/bin/bash

echo "🚀 Building NaviGrab Web Server..."
echo "================================="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📋 Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "🔨 Building project..."
make -j$(nproc)

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"
echo

# Start the web server
echo "🌐 Starting Web Server..."
echo "📡 Server will be available at: http://localhost:8080"
echo "🛑 Press Ctrl+C to stop the server"
echo

cd bin
./web_server

