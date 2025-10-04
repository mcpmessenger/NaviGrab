#pragma once

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

#include "chromium_playwright/screenshot_capture.h"
#include "chromium_playwright/dom/blink_dom_agent.h"
#include "chromium_playwright/network/http_client.h"

namespace chromium_playwright::server {

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
};

struct HTTPResponse {
    int status_code = 200;
    std::string body;
    std::map<std::string, std::string> headers;
};

class WebServer {
public:
    WebServer(int port = 8080);
    ~WebServer();

    // Server control
    bool Start();
    void Stop();
    bool IsRunning() const { return running_; }

    // API endpoints
    void RegisterScreenshotEndpoints();
    void RegisterDOMEndpoints();
    void RegisterTooltipEndpoints();
    
    // HTTP parsing utilities
    HTTPRequest ParseHTTPRequest(const std::string& request);
    std::string FormatHTTPResponse(const HTTPResponse& response);

private:
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    // Core components
    std::unique_ptr<chromium_playwright::ScreenshotCapture> screenshot_capture_;
    std::unique_ptr<chromium_playwright::dom::BlinkDOMAgent> dom_agent_;
    std::unique_ptr<chromium_playwright::network::HTTPClient> http_client_;

    // Request handling
    HTTPResponse HandleRequest(const HTTPRequest& request);
    HTTPResponse HandleScreenshotRequest(const HTTPRequest& request);
    HTTPResponse HandleDOMRequest(const HTTPRequest& request);
    HTTPResponse HandleTooltipRequest(const HTTPRequest& request);
    
    // Utility functions
    std::string ParseJSON(const std::string& json, const std::string& key);
    std::string CreateJSONResponse(const std::string& message, bool success = true);
    std::string CreateErrorResponse(const std::string& error);
    std::string CreateScreenshotResponse(const std::string& file_path, bool success);
    
    // Server thread function
    void ServerLoop();
    
    // CORS headers
    void AddCORSHeaders(HTTPResponse& response);
};

} // namespace chromium_playwright::server

