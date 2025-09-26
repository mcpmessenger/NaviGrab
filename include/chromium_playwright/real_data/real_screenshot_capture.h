#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <set>
#include "screenshot_capture.h"

namespace chromium_playwright::real_data {

// Real Screenshot Capture Interface
class ScreenshotCapture {
public:
    virtual ~ScreenshotCapture() = default;
    
    // Real screenshot capture methods
    virtual ScreenshotResult CapturePage(const std::string& url, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureElement(const std::string& url, const std::string& selector, const ScreenshotOptions& options = {}) = 0;
};

// Real Web Scraping Interface
class RealWebScraper {
public:
    virtual ~RealWebScraper() = default;
    
    struct ScrapingResult {
        std::string url;
        std::string title;
        std::string content;
        std::vector<std::string> links;
        std::map<std::string, std::string> metadata;
        std::string screenshot_path;
        bool success = false;
        std::string error_message;
    };
    
    virtual std::vector<ScrapingResult> ScrapeWebsite(const std::string& start_url, int max_depth = 3) = 0;
};

// Factory functions
std::unique_ptr<ScreenshotCapture> CreateRealScreenshotCapture();
std::unique_ptr<RealWebScraper> CreateRealWebScraper();

} // namespace chromium_playwright::real_data
