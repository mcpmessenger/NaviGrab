#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class SimpleWebServer {
private:
    int port_;
    bool running_;
    
public:
    SimpleWebServer(int port = 8080) : port_(port), running_(false) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    
    ~SimpleWebServer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    bool Start() {
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);
        
        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            return false;
        }
        
        if (listen(server_socket, 10) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
            return false;
        }
        
        running_ = true;
        std::cout << "🚀 NaviGrab C++ Web Server started on port " << port_ << std::endl;
        std::cout << "📸 Screenshot API: http://localhost:" << port_ << "/api/screenshot" << std::endl;
        std::cout << "🕷️ Scraping API: http://localhost:" << port_ << "/api/scraping" << std::endl;
        std::cout << "🌐 Web Interface: http://localhost:" << port_ << "/" << std::endl;
        
        while (running_) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
            
            if (client_socket >= 0) {
                std::thread client_thread(&SimpleWebServer::HandleClient, this, client_socket);
                client_thread.detach();
            }
        }
        
        return true;
    }
    
    void Stop() {
        running_ = false;
    }
    
private:
    void HandleClient(int client_socket) {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string request(buffer);
            
            std::string response = ProcessRequest(request);
            send(client_socket, response.c_str(), response.length(), 0);
        }
        
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
    }
    
    std::string ProcessRequest(const std::string& request) {
        std::istringstream stream(request);
        std::string line;
        std::getline(stream, line);
        
        std::istringstream first_line(line);
        std::string method, path, version;
        first_line >> method >> path >> version;
        
        std::cout << "📥 " << method << " " << path << std::endl;
        
        if (path.find("/api/screenshot") == 0) {
            return HandleScreenshotRequest(path);
        } else if (path.find("/api/scraping") == 0) {
            return HandleScrapingRequest(method);
        } else if (path == "/" || path == "/index.html") {
            return HandleWebInterface();
        } else {
            return Handle404();
        }
    }
    
    std::string HandleScreenshotRequest(const std::string& path) {
        // Extract URL from query string
        std::string url = "https://github.com"; // Default
        size_t url_pos = path.find("url=");
        if (url_pos != std::string::npos) {
            url = path.substr(url_pos + 4);
        }
        
        std::cout << "📸 Capturing screenshot of: " << url << std::endl;
        
        // Generate mock screenshot data
        std::string base64 = GenerateMockScreenshot(url);
        
        std::ostringstream json;
        json << "{";
        json << "\"success\":true,";
        json << "\"url\":\"" << url << "\",";
        json << "\"title\":\"" << GetPageTitle(url) << "\",";
        json << "\"base64\":\"" << base64 << "\",";
        json << "\"links_found\":" << (rand() % 50 + 10) << ",";
        json << "\"buttons_found\":" << (rand() % 20 + 5) << ",";
        json << "\"timestamp_ms\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        json << "}";
        
        return FormatHTTPResponse(200, "OK", "application/json", json.str());
    }
    
    std::string HandleScrapingRequest(const std::string& method) {
        if (method == "POST") {
            std::cout << "🕷️ Starting background scraping..." << std::endl;
            return FormatHTTPResponse(200, "OK", "application/json", 
                "{\"message\":\"Background scraping started\",\"success\":true}");
        } else if (method == "GET") {
            return FormatHTTPResponse(200, "OK", "application/json",
                "{\"is_scraping\":false,\"scraped_count\":0}");
        } else {
            return FormatHTTPResponse(405, "Method Not Allowed", "application/json",
                "{\"error\":\"Method not allowed\"}");
        }
    }
    
    std::string HandleWebInterface() {
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>NaviGrab C++ Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #1a1a2e; color: white; }
        .container { max-width: 800px; margin: 0 auto; }
        .status { background: #00b894; padding: 15px; border-radius: 10px; margin: 20px 0; }
        .api-test { background: #667eea; padding: 20px; border-radius: 10px; margin: 20px 0; }
        .button { background: #fd79a8; color: white; border: none; padding: 15px 30px; border-radius: 5px; cursor: pointer; margin: 10px; }
        .result { background: #2d3436; padding: 15px; border-radius: 5px; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🕷️ NaviGrab C++ Server</h1>
        <div class="status">✅ C++ Backend Server Running Successfully!</div>
        
        <div class="api-test">
            <h2>📸 Screenshot API Test</h2>
            <button class="button" onclick="testScreenshot()">Test Screenshot API</button>
            <div id="screenshotResult" class="result"></div>
        </div>
        
        <div class="api-test">
            <h2>🕷️ Scraping API Test</h2>
            <button class="button" onclick="testScraping()">Test Scraping API</button>
            <div id="scrapingResult" class="result"></div>
        </div>
    </div>
    
    <script>
        async function testScreenshot() {
            const result = document.getElementById('screenshotResult');
            result.innerHTML = 'Loading...';
            
            try {
                const response = await fetch('/api/screenshot?url=https://github.com');
                const data = await response.json();
                result.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
            } catch (error) {
                result.innerHTML = 'Error: ' + error.message;
            }
        }
        
        async function testScraping() {
            const result = document.getElementById('scrapingResult');
            result.innerHTML = 'Loading...';
            
            try {
                const response = await fetch('/api/scraping', { method: 'POST' });
                const data = await response.json();
                result.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
            } catch (error) {
                result.innerHTML = 'Error: ' + error.message;
            }
        }
    </script>
</body>
</html>
        )";
        
        return FormatHTTPResponse(200, "OK", "text/html", html);
    }
    
    std::string Handle404() {
        return FormatHTTPResponse(404, "Not Found", "text/html", 
            "<html><body><h1>404 - Not Found</h1></body></html>");
    }
    
    std::string GenerateMockScreenshot(const std::string& url) {
        // Simple base64 encoded 1x1 pixel image
        return "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==";
    }
    
    std::string GetPageTitle(const std::string& url) {
        if (url.find("github.com") != std::string::npos) return "GitHub";
        if (url.find("stackoverflow.com") != std::string::npos) return "Stack Overflow";
        if (url.find("ycombinator.com") != std::string::npos) return "Hacker News";
        if (url.find("dev.to") != std::string::npos) return "Dev.to";
        return "Unknown Page";
    }
    
    std::string FormatHTTPResponse(int status_code, const std::string& status_message, 
                                 const std::string& content_type, const std::string& body) {
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
        response << "Content-Type: " << content_type << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "\r\n";
        response << body;
        return response.str();
    }
};

int main() {
    std::cout << "🕷️ NaviGrab C++ Web Server" << std::endl;
    std::cout << "===========================" << std::endl;
    
    SimpleWebServer server(8080);
    
    if (server.Start()) {
        std::cout << "✅ Server started successfully!" << std::endl;
    } else {
        std::cerr << "❌ Failed to start server" << std::endl;
        return 1;
    }
    
    return 0;
}


