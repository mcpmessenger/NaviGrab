#include "navigrab_web_server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <chrono>
#include <random>
#include <algorithm>
#include <cstring>

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

namespace navigrab {

NaviGrabWebServer::NaviGrabWebServer(int port, const std::string& host)
    : port_(port), host_(host), running_(false) {
    
    // Initialize tooltip system
    tooltip_system_ = chromium_playwright::CreateTooltipSystem();
    if (tooltip_system_) {
        tooltip_system_->Initialize();
        background_scraper_ = tooltip_system_->GetBackgroundScraper();
    }
    
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

NaviGrabWebServer::~NaviGrabWebServer() {
    Stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NaviGrabWebServer::Start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    server_thread_ = std::thread(&NaviGrabWebServer::ServerLoop, this);
    
    std::cout << "🚀 NaviGrab Web Server starting on " << host_ << ":" << port_ << std::endl;
    return true;
}

bool NaviGrabWebServer::Stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    std::cout << "🛑 NaviGrab Web Server stopped" << std::endl;
    return true;
}

bool NaviGrabWebServer::IsRunning() const {
    return running_;
}

bool NaviGrabWebServer::InitializeTooltipSystem() {
    if (!tooltip_system_) {
        return false;
    }
    
    return tooltip_system_->Initialize();
}

chromium_playwright::TooltipSystem* NaviGrabWebServer::GetTooltipSystem() {
    return tooltip_system_.get();
}

bool NaviGrabWebServer::StartBackgroundScraping(const std::string& start_url) {
    if (!background_scraper_) {
        return false;
    }
    
    return background_scraper_->StartScraping(start_url, 2);
}

bool NaviGrabWebServer::StopBackgroundScraping() {
    if (!background_scraper_) {
        return false;
    }
    
    return background_scraper_->StopScraping();
}

bool NaviGrabWebServer::IsBackgroundScraping() const {
    if (!background_scraper_) {
        return false;
    }
    
    return background_scraper_->IsScraping();
}

int NaviGrabWebServer::GetScrapedCount() const {
    if (!background_scraper_) {
        return 0;
    }
    
    return background_scraper_->GetScrapedCount();
}

ScreenshotResponse NaviGrabWebServer::CaptureScreenshot(const ScreenshotRequest& request) {
    // Check cache first
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = screenshot_cache_.find(request.url);
        if (it != screenshot_cache_.end()) {
            return it->second;
        }
    }
    
    // Process screenshot request
    ScreenshotResponse response = ProcessScreenshotRequest(request.url, request.title);
    
    // Cache the response
    CacheScreenshot(request.url, response);
    
    return response;
}

ScreenshotResponse NaviGrabWebServer::GetCachedScreenshot(const std::string& url) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = screenshot_cache_.find(url);
    if (it != screenshot_cache_.end()) {
        return it->second;
    }
    
    ScreenshotResponse empty_response;
    empty_response.success = false;
    empty_response.error_message = "Screenshot not found in cache";
    return empty_response;
}

bool NaviGrabWebServer::CacheScreenshot(const std::string& url, const ScreenshotResponse& response) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    screenshot_cache_[url] = response;
    return true;
}

void NaviGrabWebServer::ServerLoop() {
#ifdef _WIN32
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(host_.c_str());
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(server_socket);
        return;
    }
    
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(server_socket);
        return;
    }
    
    std::cout << "✅ Server listening on " << host_ << ":" << port_ << std::endl;
    
    while (running_) {
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket != INVALID_SOCKET) {
            std::thread client_thread(&NaviGrabWebServer::HandleClient, this, client_socket);
            client_thread.detach();
        }
    }
    
    closesocket(server_socket);
#else
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket);
        return;
    }
    
    std::cout << "✅ Server listening on " << host_ << ":" << port_ << std::endl;
    
    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket >= 0) {
            std::thread client_thread(&NaviGrabWebServer::HandleClient, this, client_socket);
            client_thread.detach();
        }
    }
    
    close(server_socket);
#endif
}

void NaviGrabWebServer::HandleClient(int client_socket) {
    char buffer[4096];
    std::string request_data;
    
    // Read request
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        request_data = buffer;
    }
    
    if (request_data.empty()) {
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }
    
    // Parse request
    HTTPRequest request = ParseHTTPRequest(request_data);
    
    // Handle request
    HTTPResponse response;
    if (request.path.find("/api/screenshot") == 0) {
        response = HandleScreenshotRequest(request);
    } else if (request.path.find("/api/scraping") == 0) {
        response = HandleScrapingRequest(request);
    } else if (request.path.find("/api/tooltip") == 0) {
        response = HandleTooltipRequest(request);
    } else if (request.path.find("/api/") == 0) {
        response = HandleAPIRequest(request);
    } else {
        response = HandleStaticFile(request);
    }
    
    // Send response
    std::string response_str = FormatHTTPResponse(response);
    send(client_socket, response_str.c_str(), response_str.length(), 0);
    
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

HTTPRequest NaviGrabWebServer::ParseHTTPRequest(const std::string& raw_request) {
    HTTPRequest request;
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse first line (method, path, version)
    if (std::getline(stream, line)) {
        std::istringstream first_line(line);
        first_line >> request.method >> request.path;
        
        // Extract query string
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query_string = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            request.headers[key] = value;
        }
    }
    
    // Parse body
    std::string body_line;
    while (std::getline(stream, body_line)) {
        request.body += body_line + "\n";
    }
    
    return request;
}

HTTPResponse NaviGrabWebServer::FormatHTTPResponse(const HTTPResponse& response) {
    std::ostringstream stream;
    
    // Status line
    stream << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";
    
    // Headers
    for (const auto& header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    // Content length
    stream << "Content-Length: " << response.body.length() << "\r\n";
    
    // End headers
    stream << "\r\n";
    
    // Body
    stream << response.body;
    
    return stream.str();
}

HTTPResponse NaviGrabWebServer::HandleScreenshotRequest(const HTTPRequest& request) {
    HTTPResponse response;
    response.status_code = 200;
    response.status_message = "OK";
    response.headers["Content-Type"] = "application/json";
    
    // Parse query parameters
    std::string url = request.query_string;
    std::string title = "Unknown Page";
    
    // Extract URL from query string
    size_t url_pos = url.find("url=");
    if (url_pos != std::string::npos) {
        url = url.substr(url_pos + 4);
        // URL decode
        url = UrlDecode(url);
    }
    
    if (url.empty()) {
        response.status_code = 400;
        response.body = CreateErrorResponse("Missing URL parameter");
        return response;
    }
    
    // Create screenshot request
    ScreenshotRequest screenshot_request;
    screenshot_request.url = url;
    screenshot_request.title = title;
    screenshot_request.width = 800;
    screenshot_request.height = 600;
    screenshot_request.full_page = true;
    screenshot_request.delay_ms = 1000;
    
    // Capture screenshot
    ScreenshotResponse screenshot_response = CaptureScreenshot(screenshot_request);
    
    // Create JSON response
    std::ostringstream json;
    json << "{";
    json << "\"success\":" << (screenshot_response.success ? "true" : "false") << ",";
    json << "\"url\":\"" << screenshot_response.url << "\",";
    json << "\"title\":\"" << screenshot_response.title << "\",";
    json << "\"base64\":\"" << screenshot_response.base64_data << "\",";
    json << "\"links_found\":" << screenshot_response.links_found << ",";
    json << "\"buttons_found\":" << screenshot_response.buttons_found << ",";
    json << "\"timestamp_ms\":" << screenshot_response.timestamp_ms;
    if (!screenshot_response.error_message.empty()) {
        json << ",\"error\":\"" << screenshot_response.error_message << "\"";
    }
    json << "}";
    
    response.body = json.str();
    return response;
}

HTTPResponse NaviGrabWebServer::HandleScrapingRequest(const HTTPRequest& request) {
    HTTPResponse response;
    response.status_code = 200;
    response.status_message = "OK";
    response.headers["Content-Type"] = "application/json";
    
    if (request.method == "POST") {
        // Start scraping
        std::string start_url = "https://github.com"; // Default
        
        if (StartBackgroundScraping(start_url)) {
            response.body = CreateJSONResponse("{\"message\":\"Background scraping started\",\"start_url\":\"" + start_url + "\"}");
        } else {
            response.status_code = 500;
            response.body = CreateErrorResponse("Failed to start background scraping");
        }
    } else if (request.method == "GET") {
        // Get scraping status
        std::ostringstream json;
        json << "{";
        json << "\"is_scraping\":" << (IsBackgroundScraping() ? "true" : "false") << ",";
        json << "\"scraped_count\":" << GetScrapedCount();
        json << "}";
        
        response.body = json.str();
    } else if (request.method == "DELETE") {
        // Stop scraping
        if (StopBackgroundScraping()) {
            response.body = CreateJSONResponse("{\"message\":\"Background scraping stopped\"}");
        } else {
            response.status_code = 500;
            response.body = CreateErrorResponse("Failed to stop background scraping");
        }
    }
    
    return response;
}

HTTPResponse NaviGrabWebServer::HandleTooltipRequest(const HTTPRequest& request) {
    HTTPResponse response;
    response.status_code = 200;
    response.status_message = "OK";
    response.headers["Content-Type"] = "application/json";
    
    // Get tooltip data for URL
    std::string url = request.query_string;
    size_t url_pos = url.find("url=");
    if (url_pos != std::string::npos) {
        url = url.substr(url_pos + 4);
        url = UrlDecode(url);
    }
    
    if (url.empty()) {
        response.status_code = 400;
        response.body = CreateErrorResponse("Missing URL parameter");
        return response;
    }
    
    // Get cached screenshot
    ScreenshotResponse screenshot = GetCachedScreenshot(url);
    
    if (screenshot.success) {
        std::ostringstream json;
        json << "{";
        json << "\"success\":true,";
        json << "\"url\":\"" << screenshot.url << "\",";
        json << "\"title\":\"" << screenshot.title << "\",";
        json << "\"base64\":\"" << screenshot.base64_data << "\",";
        json << "\"links_found\":" << screenshot.links_found << ",";
        json << "\"buttons_found\":" << screenshot.buttons_found << ",";
        json << "\"timestamp_ms\":" << screenshot.timestamp_ms;
        json << "}";
        
        response.body = json.str();
    } else {
        response.status_code = 404;
        response.body = CreateErrorResponse("Tooltip data not found for URL: " + url);
    }
    
    return response;
}

HTTPResponse NaviGrabWebServer::HandleAPIRequest(const HTTPRequest& request) {
    HTTPResponse response;
    response.status_code = 404;
    response.status_message = "Not Found";
    response.headers["Content-Type"] = "application/json";
    response.body = CreateErrorResponse("API endpoint not found: " + request.path);
    return response;
}

HTTPResponse NaviGrabWebServer::HandleStaticFile(const HTTPRequest& request) {
    HTTPResponse response;
    
    // Determine file path
    std::string file_path = request.path;
    if (file_path == "/") {
        file_path = "/index.html";
    }
    
    // Remove leading slash
    if (file_path.front() == '/') {
        file_path = file_path.substr(1);
    }
    
    // Add web_interface prefix
    file_path = "web_interface/" + file_path;
    
    // Check if file exists
    if (!FileExists(file_path)) {
        response.status_code = 404;
        response.status_message = "Not Found";
        response.headers["Content-Type"] = "text/html";
        response.body = "<html><body><h1>404 - File Not Found</h1></body></html>";
        return response;
    }
    
    // Read file
    std::string content = ReadFile(file_path);
    if (content.empty()) {
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "text/html";
        response.body = "<html><body><h1>500 - Internal Server Error</h1></body></html>";
        return response;
    }
    
    // Set response
    response.status_code = 200;
    response.status_message = "OK";
    response.headers["Content-Type"] = GetMimeType(file_path);
    response.body = content;
    
    return response;
}

ScreenshotResponse NaviGrabWebServer::ProcessScreenshotRequest(const std::string& url, const std::string& title) {
    ScreenshotResponse response;
    response.url = url;
    response.title = title;
    response.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    try {
        // In a full implementation, this would:
        // 1. Navigate to URL using Chromium
        // 2. Wait for page load
        // 3. Capture screenshot using DevTools Protocol
        // 4. Extract page metadata
        
        // For now, we'll simulate the process
        std::cout << "📸 Processing screenshot request for: " << url << std::endl;
        
        // Simulate screenshot capture
        response.base64_data = GenerateMockScreenshot(url, title);
        response.links_found = CountPageLinks(url);
        response.buttons_found = CountPageButtons(url);
        response.success = true;
        
        std::cout << "✅ Screenshot processed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error processing screenshot: " << e.what() << std::endl;
        response.success = false;
        response.error_message = e.what();
    }
    
    return response;
}

std::string NaviGrabWebServer::GenerateMockScreenshot(const std::string& url, const std::string& title) {
    // This would be replaced with actual screenshot capture in a full implementation
    // For now, return a simple base64 encoded image
    
    // Create a simple 1x1 pixel PNG in base64
    return "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==";
}

int NaviGrabWebServer::CountPageLinks(const std::string& url) {
    // Simulate link counting
    if (url.find("github.com") != std::string::npos) return 25;
    if (url.find("stackoverflow.com") != std::string::npos) return 18;
    if (url.find("ycombinator.com") != std::string::npos) return 15;
    if (url.find("dev.to") != std::string::npos) return 20;
    return 10;
}

int NaviGrabWebServer::CountPageButtons(const std::string& url) {
    // Simulate button counting
    if (url.find("github.com") != std::string::npos) return 8;
    if (url.find("stackoverflow.com") != std::string::npos) return 6;
    if (url.find("ycombinator.com") != std::string::npos) return 5;
    if (url.find("dev.to") != std::string::npos) return 7;
    return 5;
}

std::string NaviGrabWebServer::UrlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string NaviGrabWebServer::GetMimeType(const std::string& file_path) {
    std::string ext = GetFileExtension(file_path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    
    return "text/plain";
}

std::string NaviGrabWebServer::GetFileExtension(const std::string& file_path) {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        return file_path.substr(dot_pos + 1);
    }
    return "";
}

bool NaviGrabWebServer::FileExists(const std::string& file_path) {
    return std::filesystem::exists(file_path);
}

std::string NaviGrabWebServer::ReadFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Utility functions
std::string CreateJSONResponse(const std::string& data) {
    return data;
}

std::string CreateErrorResponse(const std::string& error_message, int status_code) {
    std::ostringstream json;
    json << "{\"error\":\"" << error_message << "\",\"status_code\":" << status_code << "}";
    return json.str();
}

} // namespace navigrab


