#include "navigrab_web_server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace navigrab;

int main() {
    std::cout << "🚀 NaviGrab Web Server Test" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Create web server
    NaviGrabWebServer server(8080, "localhost");
    
    // Initialize tooltip system
    if (!server.InitializeTooltipSystem()) {
        std::cerr << "❌ Failed to initialize tooltip system" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Tooltip system initialized" << std::endl;
    
    // Start server
    if (!server.Start()) {
        std::cerr << "❌ Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Server started successfully" << std::endl;
    std::cout << "🌐 Server running on http://localhost:8080" << std::endl;
    std::cout << "📸 Screenshot API: http://localhost:8080/api/screenshot?url=<URL>" << std::endl;
    std::cout << "🕷️ Scraping API: http://localhost:8080/api/scraping" << std::endl;
    std::cout << "💡 Tooltip API: http://localhost:8080/api/tooltip?url=<URL>" << std::endl;
    std::cout << std::endl;
    
    // Test screenshot capture
    std::cout << "📸 Testing screenshot capture..." << std::endl;
    
    ScreenshotRequest request;
    request.url = "https://github.com";
    request.title = "GitHub";
    request.width = 800;
    request.height = 600;
    request.full_page = true;
    request.delay_ms = 1000;
    
    ScreenshotResponse response = server.CaptureScreenshot(request);
    
    if (response.success) {
        std::cout << "✅ Screenshot captured successfully" << std::endl;
        std::cout << "   URL: " << response.url << std::endl;
        std::cout << "   Title: " << response.title << std::endl;
        std::cout << "   Links: " << response.links_found << std::endl;
        std::cout << "   Buttons: " << response.buttons_found << std::endl;
        std::cout << "   Base64 length: " << response.base64_data.length() << std::endl;
    } else {
        std::cerr << "❌ Screenshot capture failed: " << response.error_message << std::endl;
    }
    
    // Test background scraping
    std::cout << std::endl;
    std::cout << "🕷️ Testing background scraping..." << std::endl;
    
    if (server.StartBackgroundScraping("https://github.com")) {
        std::cout << "✅ Background scraping started" << std::endl;
        
        // Wait for some scraping to happen
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        int scraped_count = server.GetScrapedCount();
        std::cout << "📊 Scraped " << scraped_count << " pages" << std::endl;
        
        if (server.StopBackgroundScraping()) {
            std::cout << "✅ Background scraping stopped" << std::endl;
        } else {
            std::cerr << "❌ Failed to stop background scraping" << std::endl;
        }
    } else {
        std::cerr << "❌ Failed to start background scraping" << std::endl;
    }
    
    // Test cached screenshot retrieval
    std::cout << std::endl;
    std::cout << "💾 Testing cached screenshot retrieval..." << std::endl;
    
    ScreenshotResponse cached_response = server.GetCachedScreenshot("https://github.com");
    if (cached_response.success) {
        std::cout << "✅ Cached screenshot retrieved successfully" << std::endl;
        std::cout << "   Title: " << cached_response.title << std::endl;
        std::cout << "   Links: " << cached_response.links_found << std::endl;
        std::cout << "   Buttons: " << cached_response.buttons_found << std::endl;
    } else {
        std::cout << "⚠️ No cached screenshot found (this is expected for first run)" << std::endl;
    }
    
    // Keep server running
    std::cout << std::endl;
    std::cout << "🔄 Server is running... Press Ctrl+C to stop" << std::endl;
    std::cout << "🌐 Open http://localhost:8080 in your browser to test the web interface" << std::endl;
    std::cout << "📸 Test screenshot API: http://localhost:8080/api/screenshot?url=https://github.com" << std::endl;
    
    // Keep running until interrupted
    while (server.IsRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "🛑 Server stopped" << std::endl;
    return 0;
}


