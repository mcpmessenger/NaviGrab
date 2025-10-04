#include "web_server.h"
#include <iostream>
#include <signal.h>
#include <memory>

std::unique_ptr<chromium_playwright::server::WebServer> g_server;

void signalHandler(int signum) {
    std::cout << "\n🛑 Received signal " << signum << ", shutting down server..." << std::endl;
    if (g_server) {
        g_server->Stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    std::cout << "🚀 NaviGrab Web Server - C++ Playwright Clone" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Parse command line arguments
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    // Create and start server
    g_server = std::make_unique<chromium_playwright::server::WebServer>(port);
    
    if (!g_server->Start()) {
        std::cerr << "❌ Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "\n🌐 Server is running!" << std::endl;
    std::cout << "📡 API Endpoints:" << std::endl;
    std::cout << "   POST /api/screenshot - Capture screenshots" << std::endl;
    std::cout << "   POST /api/dom - DOM interactions" << std::endl;
    std::cout << "   POST /api/tooltip - Tooltip previews" << std::endl;
    std::cout << "   GET  /api/status - Server status" << std::endl;
    std::cout << "\n💡 Open http://localhost:" << port << " in your browser" << std::endl;
    std::cout << "🛑 Press Ctrl+C to stop the server" << std::endl;
    
    // Keep server running
    while (g_server->IsRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "✅ Server stopped gracefully" << std::endl;
    return 0;
}

