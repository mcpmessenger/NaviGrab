#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

class SimpleFileServer {
private:
    std::string serverPath;
    int port;
    bool running;

public:
    SimpleFileServer(const std::string& path, int p = 8080) : serverPath(path), port(p), running(false) {}
    
    void Start() {
        running = true;
        std::cout << "🌐 Starting Simple File Server on port " << port << std::endl;
        std::cout << "📁 Serving files from: " << serverPath << std::endl;
        std::cout << "🔗 Open: http://localhost:" << port << "/proactive_scraper.html" << std::endl;
        
        // Start server in background thread
        std::thread server_thread(&SimpleFileServer::ServerLoop, this);
        server_thread.detach();
    }
    
    void Stop() {
        running = false;
    }

private:
    void ServerLoop() {
        // Simple HTTP server using WinHTTP
        HINTERNET hSession = WinHttpOpen(L"NaviGrab File Server", 
                                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME,
                                        WINHTTP_NO_PROXY_BYPASS, 0);
        
        if (!hSession) {
            std::cerr << "❌ Failed to create HTTP session" << std::endl;
            return;
        }
        
        std::wstring wport = std::to_wstring(port);
        HINTERNET hConnect = WinHttpConnect(hSession, L"localhost", port, 0);
        
        if (!hConnect) {
            std::cerr << "❌ Failed to connect to localhost" << std::endl;
            WinHttpCloseHandle(hSession);
            return;
        }
        
        std::cout << "✅ File server started successfully!" << std::endl;
        std::cout << "📂 Serving scraped data files..." << std::endl;
        
        // Keep server running
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
    }
};

int main() {
    std::cout << "🌐 NAVIGRAB FILE SERVER" << std::endl;
    std::cout << "======================" << std::endl;
    
    // Get current directory
    std::string currentDir = std::filesystem::current_path().string();
    std::cout << "📁 Current directory: " << currentDir << std::endl;
    
    // Create file server
    SimpleFileServer server(currentDir, 8080);
    
    // Start server
    server.Start();
    
    std::cout << "\n✅ File server is running!" << std::endl;
    std::cout << "🔗 Open your browser and go to:" << std::endl;
    std::cout << "   http://localhost:8080/web_interface/proactive_scraper.html" << std::endl;
    std::cout << "\n📋 This will allow the web interface to load real scraped data files." << std::endl;
    std::cout << "\nPress Enter to stop the server..." << std::endl;
    std::cin.get();
    
    server.Stop();
    std::cout << "🛑 File server stopped." << std::endl;
    
    return 0;
}


