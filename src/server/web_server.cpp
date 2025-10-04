#include "web_server.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <fstream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <thread>
#include <functional>

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

namespace chromium_playwright::server {

WebServer::WebServer(int port) : port_(port), running_(false) {
    // Initialize core components with real implementations
    screenshot_capture_ = std::make_unique<chromium_playwright::real_data::RealScreenshotCapture>();
    dom_agent_ = std::make_unique<chromium_playwright::dom::BlinkDOMAgentImpl>();
    http_client_ = std::make_unique<chromium_playwright::network::HTTPClientImpl>();
    
    std::cout << "🌐 Web Server initialized on port " << port_ << std::endl;
}

WebServer::~WebServer() {
    Stop();
}

bool WebServer::Start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    server_thread_ = std::thread(&WebServer::ServerLoop, this);
    
    std::cout << "🚀 Web Server started on http://localhost:" << port_ << std::endl;
    return true;
}

void WebServer::Stop() {
    if (running_) {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        std::cout << "🛑 Web Server stopped" << std::endl;
    }
}

void WebServer::ServerLoop() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "❌ Failed to create socket" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "❌ Failed to bind to port " << port_ << std::endl;
        return;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "❌ Failed to listen" << std::endl;
        return;
    }

    std::cout << "✅ Server listening on port " << port_ << std::endl;

    while (running_) {
        int addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        
        if (new_socket < 0) {
            if (running_) {
                std::cerr << "❌ Accept failed" << std::endl;
            }
            continue;
        }

        // Handle request in a simple way (for now, single-threaded)
        char buffer[4096] = {0};
        int valread = read(new_socket, buffer, 4096);
        
        if (valread > 0) {
            std::string request_str(buffer, valread);
            HTTPRequest request = ParseHTTPRequest(request_str);
            HTTPResponse response = HandleRequest(request);
            
            std::string response_str = FormatHTTPResponse(response);
            send(new_socket, response_str.c_str(), response_str.length(), 0);
        }
        
        close(new_socket);
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

HTTPRequest WebServer::ParseHTTPRequest(const std::string& request_str) {
    HTTPRequest request;
    
    std::istringstream stream(request_str);
    std::string line;
    
    // Parse first line (method, path, version)
    if (std::getline(stream, line)) {
        std::istringstream first_line(line);
        first_line >> request.method >> request.path;
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            request.headers[key] = value;
        }
    }
    
    // Parse body
    std::string body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    request.body = body;
    
    return request;
}

std::string WebServer::FormatHTTPResponse(const HTTPResponse& response) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code << " OK\r\n";
    
    for (const auto& header : response.headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n" << response.body;
    return oss.str();
}

HTTPResponse WebServer::HandleRequest(const HTTPRequest& request) {
    HTTPResponse response;
    AddCORSHeaders(response);
    
    std::cout << "📡 " << request.method << " " << request.path << std::endl;
    
    // Route requests
    if (request.path.find("/api/screenshot") == 0) {
        return HandleScreenshotRequest(request);
    } else if (request.path.find("/api/dom") == 0) {
        return HandleDOMRequest(request);
    } else if (request.path.find("/api/tooltip") == 0) {
        return HandleTooltipRequest(request);
    } else if (request.path == "/api/status") {
        response.body = CreateJSONResponse("Server running", true);
        response.headers["Content-Type"] = "application/json";
    } else {
        response.status_code = 404;
        response.body = CreateErrorResponse("Endpoint not found");
        response.headers["Content-Type"] = "application/json";
    }
    
    return response;
}

HTTPResponse WebServer::HandleScreenshotRequest(const HTTPRequest& request) {
    HTTPResponse response;
    AddCORSHeaders(response);
    
    if (request.method == "POST") {
        std::string url = ParseJSON(request.body, "url");
        std::string type = ParseJSON(request.body, "type");
        std::string format = ParseJSON(request.body, "format");
        
        if (url.empty()) {
            response.status_code = 400;
            response.body = CreateErrorResponse("URL is required");
            response.headers["Content-Type"] = "application/json";
            return response;
        }
        
        // Use real screenshot capture
        chromium_playwright::ScreenshotOptions options;
        options.type = format.empty() ? "png" : format;
        
        auto result = screenshot_capture_->CapturePage(url, options);
        
        if (result.success) {
            response.body = CreateScreenshotResponse(result.file_path, true);
            std::cout << "✅ Screenshot captured: " << result.file_path << std::endl;
        } else {
            response.status_code = 500;
            response.body = CreateErrorResponse(result.error_message);
            std::cerr << "❌ Screenshot failed: " << result.error_message << std::endl;
        }
    } else {
        response.status_code = 405;
        response.body = CreateErrorResponse("Method not allowed");
    }
    
    response.headers["Content-Type"] = "application/json";
    return response;
}

HTTPResponse WebServer::HandleTooltipRequest(const HTTPRequest& request) {
    HTTPResponse response;
    AddCORSHeaders(response);
    
    if (request.method == "POST") {
        std::string element_id = ParseJSON(request.body, "element_id");
        std::string interaction_type = ParseJSON(request.body, "interaction_type");
        
        if (element_id.empty()) {
            response.status_code = 400;
            response.body = CreateErrorResponse("element_id is required");
            response.headers["Content-Type"] = "application/json";
            return response;
        }
        
        // Use real tooltip capture
        auto result = screenshot_capture_->CaptureTooltipPreview(element_id, interaction_type);
        
        if (result.success) {
            response.body = CreateScreenshotResponse(result.file_path, true);
            std::cout << "✅ Tooltip preview captured: " << result.file_path << std::endl;
        } else {
            response.status_code = 500;
            response.body = CreateErrorResponse(result.error_message);
            std::cerr << "❌ Tooltip preview failed: " << result.error_message << std::endl;
        }
    } else {
        response.status_code = 405;
        response.body = CreateErrorResponse("Method not allowed");
    }
    
    response.headers["Content-Type"] = "application/json";
    return response;
}

HTTPResponse WebServer::HandleDOMRequest(const HTTPRequest& request) {
    HTTPResponse response;
    AddCORSHeaders(response);
    
    if (request.method == "POST") {
        std::string action = ParseJSON(request.body, "action");
        std::string selector = ParseJSON(request.body, "selector");
        
        if (action.empty() || selector.empty()) {
            response.status_code = 400;
            response.body = CreateErrorResponse("action and selector are required");
            response.headers["Content-Type"] = "application/json";
            return response;
        }
        
        bool success = false;
        std::string message;
        
        if (action == "click") {
            success = dom_agent_->ClickElement(selector);
            message = success ? "Element clicked successfully" : "Failed to click element";
        } else if (action == "type") {
            std::string text = ParseJSON(request.body, "text");
            success = dom_agent_->TypeText(selector, text);
            message = success ? "Text typed successfully" : "Failed to type text";
        } else if (action == "hover") {
            success = dom_agent_->HoverElement(selector);
            message = success ? "Element hovered successfully" : "Failed to hover element";
        } else {
            response.status_code = 400;
            response.body = CreateErrorResponse("Invalid action");
            response.headers["Content-Type"] = "application/json";
            return response;
        }
        
        response.body = CreateJSONResponse(message, success);
        std::cout << "✅ DOM action '" << action << "' on '" << selector << "': " << (success ? "success" : "failed") << std::endl;
    } else {
        response.status_code = 405;
        response.body = CreateErrorResponse("Method not allowed");
    }
    
    response.headers["Content-Type"] = "application/json";
    return response;
}

std::string WebServer::ParseJSON(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    
    return "";
}

std::string WebServer::CreateJSONResponse(const std::string& message, bool success) {
    return "{\"success\": " + (success ? "true" : "false") + ", \"message\": \"" + message + "\"}";
}

std::string WebServer::CreateErrorResponse(const std::string& error) {
    return "{\"success\": false, \"error\": \"" + error + "\"}";
}

std::string WebServer::CreateScreenshotResponse(const std::string& file_path, bool success) {
    return "{\"success\": " + (success ? "true" : "false") + ", \"file_path\": \"" + file_path + "\"}";
}

void WebServer::AddCORSHeaders(HTTPResponse& response) {
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
}

} // namespace chromium_playwright::server

