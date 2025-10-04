#pragma once

#include <string>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include "chromium_playwright/tooltip_system.h"

namespace navigrab {

// HTTP request structure
struct HTTPRequest {
    std::string method;
    std::string path;
    std::string query_string;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string client_ip;
    int client_port;
};

// HTTP response structure
struct HTTPResponse {
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Screenshot request structure
struct ScreenshotRequest {
    std::string url;
    std::string title;
    int width;
    int height;
    bool full_page;
    int delay_ms;
};

// Screenshot response structure
struct ScreenshotResponse {
    bool success;
    std::string base64_data;
    std::string error_message;
    std::string url;
    std::string title;
    int links_found;
    int buttons_found;
    long long timestamp_ms;
};

// NaviGrab Web Server with real browsing capability
class NaviGrabWebServer {
private:
    int port_;
    std::string host_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    // Tooltip system
    std::unique_ptr<chromium_playwright::TooltipSystem> tooltip_system_;
    
    // Background scraper
    std::unique_ptr<chromium_playwright::TooltipBackgroundScraper> background_scraper_;
    
    // Screenshot storage
    std::map<std::string, ScreenshotResponse> screenshot_cache_;
    std::mutex cache_mutex_;
    
public:
    NaviGrabWebServer(int port = 8080, const std::string& host = "localhost");
    ~NaviGrabWebServer();
    
    // Server control
    bool Start();
    bool Stop();
    bool IsRunning() const;
    
    // Tooltip system integration
    bool InitializeTooltipSystem();
    chromium_playwright::TooltipSystem* GetTooltipSystem();
    
    // Background scraping
    bool StartBackgroundScraping(const std::string& start_url);
    bool StopBackgroundScraping();
    bool IsBackgroundScraping() const;
    int GetScrapedCount() const;
    
    // Screenshot capture
    ScreenshotResponse CaptureScreenshot(const ScreenshotRequest& request);
    ScreenshotResponse GetCachedScreenshot(const std::string& url);
    bool CacheScreenshot(const std::string& url, const ScreenshotResponse& response);
    
    // API endpoints
    HTTPResponse HandleScreenshotRequest(const HTTPRequest& request);
    HTTPResponse HandleScrapingRequest(const HTTPRequest& request);
    HTTPResponse HandleTooltipRequest(const HTTPRequest& request);
    HTTPResponse HandleStaticFile(const HTTPRequest& request);
    HTTPResponse HandleAPIRequest(const HTTPRequest& request);
    
    // HTTP server
    void ServerLoop();
    HTTPRequest ParseHTTPRequest(const std::string& raw_request);
    HTTPResponse FormatHTTPResponse(const HTTPResponse& response);
    std::string GetMimeType(const std::string& file_path);
    
    // Utility functions
    std::string UrlDecode(const std::string& str);
    std::string GenerateScreenshotId();
    std::string GetCurrentTimestamp();
    
private:
    // Internal server methods
    void HandleClient(int client_socket);
    std::string ReadFile(const std::string& file_path);
    bool FileExists(const std::string& file_path);
    std::string GetFileExtension(const std::string& file_path);
    
    // Screenshot processing
    ScreenshotResponse ProcessScreenshotRequest(const std::string& url, const std::string& title);
    std::string ExtractPageTitle(const std::string& url);
    int CountPageLinks(const std::string& url);
    int CountPageButtons(const std::string& url);
    
    // Background scraping
    void BackgroundScrapingLoop();
    void ScrapeUrl(const std::string& url, int depth);
};

// Utility functions
std::string HTTPRequestToString(const HTTPRequest& request);
std::string HTTPResponseToString(const HTTPResponse& response);
std::string CreateJSONResponse(const std::string& data);
std::string CreateErrorResponse(const std::string& error_message, int status_code = 500);

} // namespace navigrab


