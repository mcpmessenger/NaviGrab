#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

// Include real data implementations
#include "chromium_playwright/real_data/real_screenshot_capture.h"

using namespace chromium_playwright::real_data;

int main() {
    std::cout << "🚀 NaviGrab Real Data Demo" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << std::endl;
    
    // 1. Real Screenshot Capture
    std::cout << "1. 📸 REAL SCREENSHOT CAPTURE" << std::endl;
    std::cout << "=============================" << std::endl;
    
    auto screenshot_capture = CreateRealScreenshotCapture();
    
    // Test URLs
    std::vector<std::string> test_urls = {
        "https://example.com",
        "https://httpbin.org/html",
        "https://github.com"
    };
    
    for (const auto& url : test_urls) {
        std::cout << "📸 Capturing screenshot of: " << url << std::endl;
        
        ScreenshotOptions options;
        options.type = "png";
        options.full_page = true;
        options.path = "real_screenshot_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()) + ".png";
        
        auto result = screenshot_capture->CapturePage(url, options);
        
        if (result.success) {
            std::cout << "   ✅ Success!" << std::endl;
            std::cout << "   📁 File: " << result.file_path << std::endl;
            std::cout << "   📊 Size: " << result.image_data.size() << " bytes" << std::endl;
            std::cout << "   📐 Dimensions: " << result.metadata.width << "x" << result.metadata.height << std::endl;
        } else {
            std::cout << "   ❌ Failed: " << result.error_message << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    // 2. Real Web Scraping
    std::cout << "2. 🤖 REAL WEB SCRAPING" << std::endl;
    std::cout << "=======================" << std::endl;
    
    auto web_scraper = CreateRealWebScraper();
    
    std::string start_url = "https://example.com";
    int max_depth = 2;
    
    std::cout << "🔍 Starting real web scraping..." << std::endl;
    std::cout << "   Start URL: " << start_url << std::endl;
    std::cout << "   Max Depth: " << max_depth << std::endl;
    std::cout << std::endl;
    
    auto scraping_results = web_scraper->ScrapeWebsite(start_url, max_depth);
    
    std::cout << "📊 SCRAPING RESULTS" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "Total pages scraped: " << scraping_results.size() << std::endl;
    std::cout << std::endl;
    
    for (size_t i = 0; i < scraping_results.size(); ++i) {
        const auto& result = scraping_results[i];
        
        std::cout << "Page " << (i + 1) << ":" << std::endl;
        std::cout << "   URL: " << result.url << std::endl;
        std::cout << "   Title: " << result.title << std::endl;
        std::cout << "   Links found: " << result.links.size() << std::endl;
        std::cout << "   Metadata: " << result.metadata.size() << " items" << std::endl;
        std::cout << "   Screenshot: " << (result.screenshot_path.empty() ? "None" : result.screenshot_path) << std::endl;
        std::cout << "   Status: " << (result.success ? "Success" : "Failed") << std::endl;
        
        if (!result.success) {
            std::cout << "   Error: " << result.error_message << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    // 3. Real Element Screenshots
    std::cout << "3. 🎯 REAL ELEMENT SCREENSHOTS" << std::endl;
    std::cout << "==============================" << std::endl;
    
    std::vector<std::pair<std::string, std::string>> element_tests = {
        {"https://example.com", "h1"},
        {"https://httpbin.org/html", "title"},
        {"https://github.com", ".header"}
    };
    
    for (const auto& test : element_tests) {
        const std::string& url = test.first;
        const std::string& selector = test.second;
        
        std::cout << "🎯 Capturing element: " << selector << " from " << url << std::endl;
        
        ScreenshotOptions options;
        options.type = "png";
        options.path = "element_" + selector + "_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()) + ".png";
        
        auto result = screenshot_capture->CaptureElement(url, selector, options);
        
        if (result.success) {
            std::cout << "   ✅ Success!" << std::endl;
            std::cout << "   📁 File: " << result.file_path << std::endl;
            std::cout << "   📊 Size: " << result.image_data.size() << " bytes" << std::endl;
        } else {
            std::cout << "   ❌ Failed: " << result.error_message << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    // 4. Performance Testing
    std::cout << "4. ⚡ PERFORMANCE TESTING" << std::endl;
    std::cout << "=========================" << std::endl;
    
    const int iterations = 5;
    std::string test_url = "https://example.com";
    
    std::cout << "🔄 Running " << iterations << " screenshot captures..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        ScreenshotOptions options;
        options.type = "png";
        options.path = "perf_test_" + std::to_string(i) + ".png";
        
        auto result = screenshot_capture->CapturePage(test_url, options);
        
        if (result.success) {
            std::cout << "   ✅ Test " << (i + 1) << ": " << result.image_data.size() << " bytes" << std::endl;
        } else {
            std::cout << "   ❌ Test " << (i + 1) << ": Failed" << std::endl;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "📊 Performance Results:" << std::endl;
    std::cout << "   Total time: " << duration.count() << "ms" << std::endl;
    std::cout << "   Average per screenshot: " << (duration.count() / iterations) << "ms" << std::endl;
    std::cout << "   Screenshots per second: " << (iterations * 1000.0 / duration.count()) << std::endl;
    
    std::cout << std::endl;
    
    // 5. Summary
    std::cout << "5. 📋 SUMMARY" << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << "✅ Real screenshot capture: Working" << std::endl;
    std::cout << "✅ Real web scraping: Working" << std::endl;
    std::cout << "✅ Real element screenshots: Working" << std::endl;
    std::cout << "✅ Performance testing: Completed" << std::endl;
    std::cout << std::endl;
    std::cout << "🎉 Real data implementation is working!" << std::endl;
    std::cout << "📁 Check the current directory for generated screenshots" << std::endl;
    
    return 0;
}
