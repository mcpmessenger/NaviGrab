#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace chromium_playwright {

// Forward declarations
class Browser;
class Page;
class Element;

// Tooltip data structure
struct TooltipData {
    std::string id;
    std::string url;
    std::string title;
    std::string base64_screenshot;
    std::string description;
    int links_found;
    int buttons_found;
    std::chrono::system_clock::time_point timestamp;
    int depth;
    
    TooltipData() : links_found(0), buttons_found(0), depth(0) {}
};

// Local storage interface for tooltip data
class TooltipStorage {
public:
    virtual ~TooltipStorage() = default;
    
    // Store tooltip data
    virtual bool StoreTooltipData(const TooltipData& data) = 0;
    
    // Retrieve tooltip data by URL
    virtual std::unique_ptr<TooltipData> GetTooltipData(const std::string& url) = 0;
    
    // Get all stored tooltip data
    virtual std::vector<TooltipData> GetAllTooltipData() = 0;
    
    // Clear all data
    virtual bool ClearAllData() = 0;
    
    // Check if data exists for URL
    virtual bool HasDataForUrl(const std::string& url) = 0;
};

// Screenshot capture interface
class TooltipScreenshotCapture {
public:
    virtual ~TooltipScreenshotCapture() = default;
    
    // Capture screenshot of current page
    virtual std::string CapturePageScreenshot() = 0;
    
    // Capture screenshot of specific element
    virtual std::string CaptureElementScreenshot(const Element& element) = 0;
    
    // Capture screenshot of URL (navigate and capture)
    virtual std::string CaptureUrlScreenshot(const std::string& url) = 0;
};

// Background scraper for proactive tooltip data collection
class TooltipBackgroundScraper {
public:
    virtual ~TooltipBackgroundScraper() = default;
    
    // Start background scraping
    virtual bool StartScraping(const std::string& start_url, int max_depth = 2) = 0;
    
    // Stop background scraping
    virtual bool StopScraping() = 0;
    
    // Check if scraping is running
    virtual bool IsScraping() const = 0;
    
    // Get scraping progress
    virtual int GetScrapedCount() const = 0;
    
    // Set scraping configuration
    virtual void SetMaxDepth(int depth) = 0;
    virtual void SetMaxPages(int pages) = 0;
    virtual void SetDelay(int delay_ms) = 0;
};

// Main tooltip system interface
class TooltipSystem {
public:
    virtual ~TooltipSystem() = default;
    
    // Initialize the tooltip system
    virtual bool Initialize() = 0;
    
    // Show tooltip for element
    virtual void ShowTooltip(const Element& element, const TooltipData& data) = 0;
    
    // Hide tooltip
    virtual void HideTooltip() = 0;
    
    // Check if tooltip is visible
    virtual bool IsTooltipVisible() const = 0;
    
    // Update tooltip position
    virtual void UpdateTooltipPosition(int x, int y) = 0;
    
    // Set tooltip style
    virtual void SetTooltipStyle(const std::string& css_style) = 0;
    
    // Enable/disable tooltips
    virtual void SetEnabled(bool enabled) = 0;
    virtual bool IsEnabled() const = 0;
    
    // Get storage interface
    virtual TooltipStorage* GetStorage() = 0;
    
    // Get screenshot capture interface
    virtual TooltipScreenshotCapture* GetScreenshotCapture() = 0;
    
    // Get background scraper interface
    virtual TooltipBackgroundScraper* GetBackgroundScraper() = 0;
};

// Chrome extension integration interface
class ChromeExtensionIntegration {
public:
    virtual ~ChromeExtensionIntegration() = default;
    
    // Inject tooltip system into page
    virtual bool InjectTooltipSystem(Page& page) = 0;
    
    // Register content script
    virtual bool RegisterContentScript(const std::string& script_path) = 0;
    
    // Send message to content script
    virtual bool SendMessageToContentScript(const std::string& message) = 0;
    
    // Listen for messages from content script
    virtual void SetMessageListener(std::function<void(const std::string&)> callback) = 0;
    
    // Get extension manifest
    virtual std::string GetExtensionManifest() = 0;
};

// Factory functions
std::unique_ptr<TooltipSystem> CreateTooltipSystem();
std::unique_ptr<TooltipStorage> CreateTooltipStorage();
std::unique_ptr<TooltipScreenshotCapture> CreateTooltipScreenshotCapture();
std::unique_ptr<TooltipBackgroundScraper> CreateTooltipBackgroundScraper();
std::unique_ptr<ChromeExtensionIntegration> CreateChromeExtensionIntegration();

} // namespace chromium_playwright


