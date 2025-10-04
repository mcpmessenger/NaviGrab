#include "chromium_playwright/openai/screenshot_analyzer.h"
#include "chromium_playwright/real_data/real_screenshot_capture.h"
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace chromium_playwright::openai;

class NaviGrabOpenAIServer {
private:
    SOCKET server_socket_;
    std::unique_ptr<ScreenshotAnalyzer> analyzer_;
    std::unique_ptr<real_data::RealScreenshotCapture> screenshot_capture_;
    bool running_;
    int port_;
    
public:
    NaviGrabOpenAIServer(int port = 8080) : port_(port), running_(false) {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cout << "❌ WSAStartup failed: " << result << std::endl;
            return;
        }
        
        // Initialize components
        analyzer_ = CreateScreenshotAnalyzer();
        screenshot_capture_ = std::make_unique<real_data::RealScreenshotCapture>();
        
        // Set up OpenAI configuration (will be set via API)
        OpenAIConfig config;
        config.base_url = "https://api.openai.com/v1";
        config.model = "gpt-4-vision-preview";
        config.max_tokens = 1500;
        config.temperature = 0.3;
        analyzer_->SetOpenAIConfig(config);
        
        // Set analysis settings
        AnalysisSettings settings = AnalysisSettings::GetUIAnalysisSettings();
        analyzer_->SetAnalysisSettings(settings);
    }
    
    ~NaviGrabOpenAIServer() {
        Stop();
        WSACleanup();
    }
    
    bool Start() {
        // Create socket
        server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket_ == INVALID_SOCKET) {
            std::cout << "❌ Socket creation failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        
        // Bind socket
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);
        
        if (bind(server_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cout << "❌ Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(server_socket_);
            return false;
        }
        
        // Listen for connections
        if (listen(server_socket_, SOMAXCONN) == SOCKET_ERROR) {
            std::cout << "❌ Listen failed: " << WSAGetLastError() << std::endl;
            closesocket(server_socket_);
            return false;
        }
        
        running_ = true;
        std::cout << "✅ NaviGrab OpenAI Server started on port " << port_ << std::endl;
        std::cout << "🌐 Server endpoints:" << std::endl;
        std::cout << "   POST /api/screenshot/analyze - Analyze screenshot with OpenAI" << std::endl;
        std::cout << "   POST /api/capture-and-analyze - Capture and analyze URL" << std::endl;
        std::cout << "   POST /api/config/openai - Set OpenAI configuration" << std::endl;
        std::cout << "   GET  /api/status - Server status" << std::endl;
        
        // Start listening thread
        std::thread listen_thread(&NaviGrabOpenAIServer::ListenLoop, this);
        listen_thread.detach();
        
        return true;
    }
    
    void Stop() {
        if (running_) {
            running_ = false;
            if (server_socket_ != INVALID_SOCKET) {
                closesocket(server_socket_);
            }
            std::cout << "✅ NaviGrab OpenAI Server stopped" << std::endl;
        }
    }
    
    bool IsRunning() const {
        return running_;
    }
    
private:
    void ListenLoop() {
        while (running_) {
            sockaddr_in client_addr;
            int client_addr_len = sizeof(client_addr);
            
            SOCKET client_socket = accept(server_socket_, (sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == INVALID_SOCKET) {
                if (running_) {
                    std::cout << "❌ Accept failed: " << WSAGetLastError() << std::endl;
                }
                continue;
            }
            
            // Handle client in separate thread
            std::thread client_thread(&NaviGrabOpenAIServer::HandleClient, this, client_socket);
            client_thread.detach();
        }
    }
    
    void HandleClient(SOCKET client_socket) {
        char buffer[8192];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string request(buffer);
            
            std::string response = ProcessRequest(request);
            
            send(client_socket, response.c_str(), response.length(), 0);
        }
        
        closesocket(client_socket);
    }
    
    std::string ProcessRequest(const std::string& request) {
        std::string response;
        
        // Parse HTTP request
        std::istringstream request_stream(request);
        std::string method, path, version;
        request_stream >> method >> path >> version;
        
        std::cout << "📡 Request: " << method << " " << path << std::endl;
        
        if (method == "GET" && path == "/api/status") {
            response = HandleStatus();
        }
        else if (method == "POST" && path == "/api/screenshot/analyze") {
            response = HandleScreenshotAnalyze(request);
        }
        else if (method == "POST" && path == "/api/capture-and-analyze") {
            response = HandleCaptureAndAnalyze(request);
        }
        else if (method == "POST" && path == "/api/config/openai") {
            response = HandleOpenAIConfig(request);
        }
        else if (method == "GET" && path == "/") {
            response = HandleWebInterface();
        }
        else {
            response = CreateErrorResponse(404, "Not Found");
        }
        
        return response;
    }
    
    std::string HandleStatus() {
        std::ostringstream json;
        json << "{";
        json << "\"status\":\"running\",";
        json << "\"openai_configured\":" << (analyzer_->IsConfigured() ? "true" : "false") << ",";
        json << "\"port\":" << port_ << ",";
        json << "\"timestamp\":\"" << GetCurrentTimestamp() << "\"";
        json << "}";
        
        return CreateJSONResponse(json.str());
    }
    
    std::string HandleScreenshotAnalyze(const std::string& request) {
        try {
            // Extract base64 image from request body
            std::string base64_data = ExtractBase64FromRequest(request);
            if (base64_data.empty()) {
                return CreateErrorResponse(400, "No image data provided");
            }
            
            // Convert base64 to binary
            std::vector<uint8_t> image_data = Base64Decode(base64_data);
            
            // Analyze screenshot
            auto analysis_response = analyzer_->AnalyzeScreenshot(
                image_data, 
                ScreenshotAnalysisRequest::AnalysisType::GENERAL
            );
            
            // Create JSON response
            std::ostringstream json;
            json << "{";
            json << "\"success\":" << (analysis_response.success ? "true" : "false") << ",";
            if (!analysis_response.success) {
                json << "\"error\":\"" << EscapeJSON(analysis_response.error_message) << "\",";
            }
            json << "\"analysis\":\"" << EscapeJSON(analysis_response.analysis_text) << "\",";
            json << "\"findings\":[";
            for (size_t i = 0; i < analysis_response.key_findings.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << EscapeJSON(analysis_response.key_findings[i]) << "\"";
            }
            json << "],";
            json << "\"recommendations\":[";
            for (size_t i = 0; i < analysis_response.recommendations.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << EscapeJSON(analysis_response.recommendations[i]) << "\"";
            }
            json << "],";
            json << "\"timestamp\":\"" << GetCurrentTimestamp() << "\"";
            json << "}";
            
            return CreateJSONResponse(json.str());
            
        } catch (const std::exception& e) {
            return CreateErrorResponse(500, "Analysis failed: " + std::string(e.what()));
        }
    }
    
    std::string HandleCaptureAndAnalyze(const std::string& request) {
        try {
            // Extract URL from request body
            std::string url = ExtractURLFromRequest(request);
            if (url.empty()) {
                return CreateErrorResponse(400, "No URL provided");
            }
            
            std::cout << "📸 Capturing and analyzing: " << url << std::endl;
            
            // Capture and analyze
            auto analysis_response = analyzer_->CaptureAndAnalyze(url);
            
            // Create JSON response
            std::ostringstream json;
            json << "{";
            json << "\"success\":" << (analysis_response.success ? "true" : "false") << ",";
            if (!analysis_response.success) {
                json << "\"error\":\"" << EscapeJSON(analysis_response.error_message) << "\",";
            }
            json << "\"url\":\"" << EscapeJSON(url) << "\",";
            json << "\"analysis\":\"" << EscapeJSON(analysis_response.analysis_text) << "\",";
            json << "\"findings\":[";
            for (size_t i = 0; i < analysis_response.key_findings.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << EscapeJSON(analysis_response.key_findings[i]) << "\"";
            }
            json << "],";
            json << "\"recommendations\":[";
            for (size_t i = 0; i < analysis_response.recommendations.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << EscapeJSON(analysis_response.recommendations[i]) << "\"";
            }
            json << "],";
            json << "\"timestamp\":\"" << GetCurrentTimestamp() << "\"";
            json << "}";
            
            return CreateJSONResponse(json.str());
            
        } catch (const std::exception& e) {
            return CreateErrorResponse(500, "Capture and analyze failed: " + std::string(e.what()));
        }
    }
    
    std::string HandleOpenAIConfig(const std::string& request) {
        try {
            // Extract API key from request body
            std::string api_key = ExtractAPIKeyFromRequest(request);
            if (api_key.empty()) {
                return CreateErrorResponse(400, "No API key provided");
            }
            
            // Update OpenAI configuration
            OpenAIConfig config;
            config.api_key = api_key;
            config.base_url = "https://api.openai.com/v1";
            config.model = "gpt-4-vision-preview";
            config.max_tokens = 1500;
            config.temperature = 0.3;
            
            analyzer_->SetOpenAIConfig(config);
            
            std::cout << "✅ OpenAI API key configured" << std::endl;
            
            std::ostringstream json;
            json << "{";
            json << "\"success\":true,";
            json << "\"message\":\"OpenAI configuration updated\",";
            json << "\"timestamp\":\"" << GetCurrentTimestamp() << "\"";
            json << "}";
            
            return CreateJSONResponse(json.str());
            
        } catch (const std::exception& e) {
            return CreateErrorResponse(500, "Configuration failed: " + std::string(e.what()));
        }
    }
    
    std::string HandleWebInterface() {
        // Return a simple HTML interface for testing
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>NaviGrab OpenAI Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .section { margin: 20px 0; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }
        input, textarea, button { margin: 10px 0; padding: 10px; width: 100%; box-sizing: border-box; }
        button { background: #007bff; color: white; border: none; cursor: pointer; }
        button:hover { background: #0056b3; }
        .result { background: #f8f9fa; padding: 15px; border-radius: 5px; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🕷️ NaviGrab OpenAI Server</h1>
        <p>Server is running and ready to analyze screenshots with OpenAI Vision API.</p>
        
        <div class="section">
            <h3>🔑 Configure OpenAI API Key</h3>
            <input type="password" id="apiKey" placeholder="Enter your OpenAI API key">
            <button onclick="setAPIKey()">Set API Key</button>
            <div id="configResult" class="result" style="display:none;"></div>
        </div>
        
        <div class="section">
            <h3>📸 Capture and Analyze URL</h3>
            <input type="url" id="url" placeholder="Enter URL to capture and analyze" value="https://github.com">
            <button onclick="captureAndAnalyze()">Capture & Analyze</button>
            <div id="analysisResult" class="result" style="display:none;"></div>
        </div>
        
        <div class="section">
            <h3>📊 Server Status</h3>
            <button onclick="checkStatus()">Check Status</button>
            <div id="statusResult" class="result" style="display:none;"></div>
        </div>
    </div>
    
    <script>
        async function setAPIKey() {
            const apiKey = document.getElementById('apiKey').value;
            if (!apiKey) {
                showResult('configResult', 'Please enter an API key', 'error');
                return;
            }
            
            try {
                const response = await fetch('/api/config/openai', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ api_key: apiKey })
                });
                
                const result = await response.json();
                showResult('configResult', result.success ? 'API key configured successfully!' : result.error, result.success ? 'success' : 'error');
            } catch (error) {
                showResult('configResult', 'Error: ' + error.message, 'error');
            }
        }
        
        async function captureAndAnalyze() {
            const url = document.getElementById('url').value;
            if (!url) {
                showResult('analysisResult', 'Please enter a URL', 'error');
                return;
            }
            
            showResult('analysisResult', 'Capturing and analyzing...', 'info');
            
            try {
                const response = await fetch('/api/capture-and-analyze', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ url: url })
                });
                
                const result = await response.json();
                if (result.success) {
                    let html = '<h4>Analysis Results:</h4>';
                    html += '<p><strong>Analysis:</strong></p>';
                    html += '<div style="background: white; padding: 10px; border-radius: 5px; margin: 10px 0;">';
                    html += result.analysis.replace(/\n/g, '<br>');
                    html += '</div>';
                    
                    if (result.findings && result.findings.length > 0) {
                        html += '<p><strong>Key Findings:</strong></p><ul>';
                        result.findings.forEach(finding => {
                            html += '<li>' + finding + '</li>';
                        });
                        html += '</ul>';
                    }
                    
                    if (result.recommendations && result.recommendations.length > 0) {
                        html += '<p><strong>Recommendations:</strong></p><ul>';
                        result.recommendations.forEach(rec => {
                            html += '<li>' + rec + '</li>';
                        });
                        html += '</ul>';
                    }
                    
                    showResult('analysisResult', html, 'success');
                } else {
                    showResult('analysisResult', 'Error: ' + result.error, 'error');
                }
            } catch (error) {
                showResult('analysisResult', 'Error: ' + error.message, 'error');
            }
        }
        
        async function checkStatus() {
            try {
                const response = await fetch('/api/status');
                const result = await response.json();
                let html = '<h4>Server Status:</h4>';
                html += '<p><strong>Status:</strong> ' + result.status + '</p>';
                html += '<p><strong>OpenAI Configured:</strong> ' + (result.openai_configured ? 'Yes' : 'No') + '</p>';
                html += '<p><strong>Port:</strong> ' + result.port + '</p>';
                html += '<p><strong>Timestamp:</strong> ' + result.timestamp + '</p>';
                showResult('statusResult', html, 'success');
            } catch (error) {
                showResult('statusResult', 'Error: ' + error.message, 'error');
            }
        }
        
        function showResult(elementId, message, type) {
            const element = document.getElementById(elementId);
            element.style.display = 'block';
            element.innerHTML = message;
            element.className = 'result ' + type;
        }
    </script>
</body>
</html>
        )";
        
        return CreateHTMLResponse(html);
    }
    
    // Utility functions
    std::string ExtractBase64FromRequest(const std::string& request) {
        // Simple extraction - in real implementation, parse JSON properly
        size_t pos = request.find("\"image\":\"");
        if (pos != std::string::npos) {
            pos += 9; // length of "\"image\":\""
            size_t end_pos = request.find("\"", pos);
            if (end_pos != std::string::npos) {
                return request.substr(pos, end_pos - pos);
            }
        }
        return "";
    }
    
    std::string ExtractURLFromRequest(const std::string& request) {
        size_t pos = request.find("\"url\":\"");
        if (pos != std::string::npos) {
            pos += 7; // length of "\"url\":\""
            size_t end_pos = request.find("\"", pos);
            if (end_pos != std::string::npos) {
                return request.substr(pos, end_pos - pos);
            }
        }
        return "";
    }
    
    std::string ExtractAPIKeyFromRequest(const std::string& request) {
        size_t pos = request.find("\"api_key\":\"");
        if (pos != std::string::npos) {
            pos += 11; // length of "\"api_key\":\""
            size_t end_pos = request.find("\"", pos);
            if (end_pos != std::string::npos) {
                return request.substr(pos, end_pos - pos);
            }
        }
        return "";
    }
    
    std::vector<uint8_t> Base64Decode(const std::string& encoded) {
        // Simple base64 decode - in production, use a proper library
        std::vector<uint8_t> result;
        // Implementation would go here
        return result;
    }
    
    std::string EscapeJSON(const std::string& str) {
        std::string escaped = str;
        // Simple escaping - replace quotes and backslashes
        size_t pos = 0;
        while ((pos = escaped.find("\"", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\"");
            pos += 2;
        }
        pos = 0;
        while ((pos = escaped.find("\\", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\\");
            pos += 2;
        }
        return escaped;
    }
    
    std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
    
    std::string CreateJSONResponse(const std::string& json_body) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "Content-Length: " << json_body.length() << "\r\n";
        response << "\r\n";
        response << json_body;
        return response.str();
    }
    
    std::string CreateHTMLResponse(const std::string& html_body) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << html_body.length() << "\r\n";
        response << "\r\n";
        response << html_body;
        return response.str();
    }
    
    std::string CreateErrorResponse(int status_code, const std::string& message) {
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " " << GetStatusText(status_code) << "\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        
        std::ostringstream json_body;
        json_body << "{\"success\":false,\"error\":\"" << EscapeJSON(message) << "\"}";
        
        response << "Content-Length: " << json_body.str().length() << "\r\n";
        response << "\r\n";
        response << json_body.str();
        return response.str();
    }
    
    std::string GetStatusText(int status_code) {
        switch (status_code) {
            case 200: return "OK";
            case 400: return "Bad Request";
            case 404: return "Not Found";
            case 500: return "Internal Server Error";
            default: return "Unknown";
        }
    }
};

int main() {
    std::cout << "🚀 NaviGrab OpenAI Server" << std::endl;
    std::cout << "========================" << std::endl;
    
    NaviGrabOpenAIServer server(8080);
    
    if (!server.Start()) {
        std::cout << "❌ Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "\n💡 Open http://localhost:8080 in your browser to access the interface" << std::endl;
    std::cout << "🛑 Press Ctrl+C to stop the server" << std::endl;
    
    // Keep server running
    while (server.IsRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}

