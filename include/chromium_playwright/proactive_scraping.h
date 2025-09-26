#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <optional>
#include "browser_control.h"
#include "dom_interaction.h"
#include "screenshot_capture.h"

namespace chromium_playwright {

// Forward declarations
class Scraper;
class TraversalEngine;
class ChangeDetector;

// Scraping configuration
struct ScrapingConfig {
    std::string start_url;
    int max_depth = 5;
    std::vector<std::string> allowed_domains;
    std::vector<std::string> blocked_domains;
    std::vector<std::string> screenshot_selectors;
    std::map<std::string, std::string> data_extraction_rules;
    bool click_all_buttons = true;
    bool follow_all_links = true;
    bool fill_forms = true;
    bool take_screenshots = true;
    bool extract_data = true;
    std::chrono::milliseconds page_timeout{30000};
    std::chrono::milliseconds action_timeout{5000};
    int max_pages = 100;
    int max_actions_per_page = 50;
    std::string output_directory = "./scraped_data";
    std::map<std::string, std::string> custom_settings;
};

// Scraped page data
struct ScrapedPageData {
    std::string url;
    std::string title;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> extracted_data;
    std::vector<std::string> screenshot_paths;
    std::vector<std::string> interacted_elements;
    std::vector<std::string> navigation_sequence;
    std::string html_content;
    std::string text_content;
    std::map<std::string, std::string> metadata;
    int depth = 0;
    std::string parent_url;
    std::vector<std::string> child_urls;
    bool is_error = false;
    std::string error_message;
};

// Scraping session information
struct ScrapingSession {
    int session_id;
    ScrapingConfig config;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<ScrapedPageData> scraped_pages;
    std::set<std::string> visited_urls;
    std::set<std::string> failed_urls;
    int total_pages = 0;
    int successful_pages = 0;
    int failed_pages = 0;
    bool is_running = false;
    bool is_paused = false;
    std::string status_message;
};

// Element interaction strategy
enum class InteractionStrategy {
    CLICK_ALL,
    CLICK_VISIBLE,
    CLICK_BUTTONS_ONLY,
    CLICK_LINKS_ONLY,
    FORM_FILLING,
    CUSTOM_SELECTORS
};

// Data extraction rule
struct DataExtractionRule {
    std::string name;
    std::string selector;
    std::string attribute; // "text", "innerHTML", "value", or attribute name
    bool required = false;
    std::string default_value = "";
    std::string transform_function = ""; // JavaScript function to transform extracted data
    std::map<std::string, std::string> options;
};

// Scraping progress callback
using ScrapingProgressCallback = std::function<void(const ScrapingSession& session, const ScrapedPageData& page_data)>;

// Scraping error callback
using ScrapingErrorCallback = std::function<void(const ScrapingSession& session, const std::string& error_message)>;

// Scraper Interface
class Scraper {
public:
    virtual ~Scraper() = default;

    // Session management
    virtual int StartScraping(const ScrapingConfig& config) = 0;
    virtual bool StopScraping(int session_id) = 0;
    virtual bool PauseScraping(int session_id) = 0;
    virtual bool ResumeScraping(int session_id) = 0;
    virtual bool IsScraping(int session_id) const = 0;

    // Session information
    virtual std::vector<int> GetActiveSessions() const = 0;
    virtual std::optional<ScrapingSession> GetSession(int session_id) const = 0;
    virtual std::vector<ScrapedPageData> GetScrapingResults(int session_id) = 0;
    virtual std::vector<ScrapedPageData> GetScrapingResults(int session_id, int limit, int offset = 0) = 0;

    // Progress monitoring
    virtual void SetProgressCallback(ScrapingProgressCallback callback) = 0;
    virtual void SetErrorCallback(ScrapingErrorCallback callback) = 0;
    virtual void RemoveProgressCallback() = 0;
    virtual void RemoveErrorCallback() = 0;

    // Configuration
    virtual void SetDefaultConfig(const ScrapingConfig& config) = 0;
    virtual ScrapingConfig GetDefaultConfig() const = 0;
    virtual void UpdateSessionConfig(int session_id, const ScrapingConfig& config) = 0;

    // Data export
    virtual bool ExportToJson(int session_id, const std::string& file_path) = 0;
    virtual bool ExportToCsv(int session_id, const std::string& file_path) = 0;
    virtual bool ExportToXml(int session_id, const std::string& file_path) = 0;
    virtual std::string ExportToJsonString(int session_id) = 0;

    // Cleanup
    virtual void ClearSession(int session_id) = 0;
    virtual void ClearAllSessions() = 0;
    virtual void Shutdown() = 0;
};

// Traversal Engine Interface
class TraversalEngine {
public:
    virtual ~TraversalEngine() = default;

    // Traversal control
    virtual bool StartTraversal(const ScrapingConfig& config, ScrapingSession& session) = 0;
    virtual bool StopTraversal() = 0;
    virtual bool PauseTraversal() = 0;
    virtual bool ResumeTraversal() = 0;
    virtual bool IsTraversing() const = 0;

    // Page processing
    virtual bool ProcessPage(Page& page, ScrapedPageData& page_data) = 0;
    virtual std::vector<std::string> DiscoverLinks(Page& page) = 0;
    virtual std::vector<std::string> DiscoverButtons(Page& page) = 0;
    virtual std::vector<std::string> DiscoverForms(Page& page) = 0;

    // Interaction strategies
    virtual void SetInteractionStrategy(InteractionStrategy strategy) = 0;
    virtual InteractionStrategy GetInteractionStrategy() const = 0;
    virtual bool InteractWithElement(Page& page, const std::string& selector, const std::string& action) = 0;

    // Configuration
    virtual void SetMaxDepth(int max_depth) = 0;
    virtual int GetMaxDepth() const = 0;
    virtual void SetAllowedDomains(const std::vector<std::string>& domains) = 0;
    virtual std::vector<std::string> GetAllowedDomains() const = 0;
    virtual void SetBlockedDomains(const std::vector<std::string>& domains) = 0;
    virtual std::vector<std::string> GetBlockedDomains() const = 0;

    // Callbacks
    virtual void SetPageProcessedCallback(std::function<void(const ScrapedPageData&)> callback) = 0;
    virtual void SetLinkDiscoveredCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void SetErrorCallback(std::function<void(const std::string&)> callback) = 0;
};

// Change Detector Interface
class ChangeDetector {
public:
    virtual ~ChangeDetector() = default;

    // Change detection methods
    virtual bool HasPageChanged(Page& page, const std::string& previous_state) = 0;
    virtual bool HasElementChanged(ElementHandle& element, const std::string& previous_state) = 0;
    virtual bool HasContentChanged(const std::string& current_content, const std::string& previous_content) = 0;

    // State capture
    virtual std::string CapturePageState(Page& page) = 0;
    virtual std::string CaptureElementState(ElementHandle& element) = 0;
    virtual std::string CaptureContentState(const std::string& content) = 0;

    // Visual change detection
    virtual bool HasVisualChanged(const std::vector<uint8_t>& current_image, 
                                 const std::vector<uint8_t>& previous_image,
                                 double threshold = 0.1) = 0;
    virtual std::vector<Rect> GetChangedRegions(const std::vector<uint8_t>& current_image, 
                                               const std::vector<uint8_t>& previous_image,
                                               double threshold = 0.1) = 0;

    // Hash-based change detection
    virtual std::string ComputePageHash(Page& page) = 0;
    virtual std::string ComputeElementHash(ElementHandle& element) = 0;
    virtual std::string ComputeContentHash(const std::string& content) = 0;
    virtual std::string ComputeImageHash(const std::vector<uint8_t>& image_data) = 0;

    // Configuration
    virtual void SetChangeThreshold(double threshold) = 0;
    virtual double GetChangeThreshold() const = 0;
    virtual void SetHashAlgorithm(const std::string& algorithm) = 0;
    virtual std::string GetHashAlgorithm() const = 0;
};

// Data Extractor Interface
class DataExtractor {
public:
    virtual ~DataExtractor() = default;

    // Data extraction
    virtual std::map<std::string, std::string> ExtractData(Page& page, 
                                                          const std::vector<DataExtractionRule>& rules) = 0;
    virtual std::string ExtractDataByRule(Page& page, const DataExtractionRule& rule) = 0;
    virtual std::vector<std::map<std::string, std::string>> ExtractDataFromElements(Page& page, 
                                                                                   const std::string& selector,
                                                                                   const std::vector<DataExtractionRule>& rules) = 0;

    // Rule management
    virtual void AddExtractionRule(const DataExtractionRule& rule) = 0;
    virtual void RemoveExtractionRule(const std::string& rule_name) = 0;
    virtual void ClearExtractionRules() = 0;
    virtual std::vector<DataExtractionRule> GetExtractionRules() const = 0;

    // Validation
    virtual bool ValidateRule(const DataExtractionRule& rule) const = 0;
    virtual std::vector<std::string> ValidateRules(const std::vector<DataExtractionRule>& rules) const = 0;

    // Custom extractors
    virtual void RegisterCustomExtractor(const std::string& name, 
                                       std::function<std::string(Page&, const std::string&)> extractor) = 0;
    virtual void UnregisterCustomExtractor(const std::string& name) = 0;
    virtual std::vector<std::string> GetCustomExtractors() const = 0;
};

// Scraping Analytics Interface
class ScrapingAnalytics {
public:
    virtual ~ScrapingAnalytics() = default;

    // Session analytics
    virtual std::map<std::string, double> GetSessionMetrics(int session_id) const = 0;
    virtual std::vector<std::string> GetSessionErrors(int session_id) const = 0;
    virtual std::chrono::milliseconds GetSessionDuration(int session_id) const = 0;
    virtual int GetPagesPerMinute(int session_id) const = 0;

    // Performance metrics
    virtual double GetAveragePageLoadTime(int session_id) const = 0;
    virtual double GetAverageActionTime(int session_id) const = 0;
    virtual double GetSuccessRate(int session_id) const = 0;
    virtual std::map<std::string, int> GetActionCounts(int session_id) const = 0;

    // Data quality metrics
    virtual double GetDataCompleteness(int session_id) const = 0;
    virtual std::map<std::string, int> GetExtractionSuccessRates(int session_id) const = 0;
    virtual std::vector<std::string> GetFailedExtractions(int session_id) const = 0;

    // Export analytics
    virtual bool ExportAnalyticsToJson(int session_id, const std::string& file_path) = 0;
    virtual std::string GetAnalyticsReport(int session_id) = 0;
};

// Factory functions
std::unique_ptr<Scraper> CreateScraper();
std::unique_ptr<TraversalEngine> CreateTraversalEngine();
std::unique_ptr<ChangeDetector> CreateChangeDetector();
std::unique_ptr<DataExtractor> CreateDataExtractor();
std::unique_ptr<ScrapingAnalytics> CreateScrapingAnalytics();

// Utility functions
namespace scraping_utils {
    // Configuration validation
    bool ValidateConfig(const ScrapingConfig& config);
    std::vector<std::string> ValidateConfigErrors(const ScrapingConfig& config);
    
    // URL utilities
    bool IsUrlAllowed(const std::string& url, const std::vector<std::string>& allowed_domains, 
                     const std::vector<std::string>& blocked_domains);
    std::string NormalizeUrl(const std::string& url);
    std::string GetDomainFromUrl(const std::string& url);
    
    // Data processing
    std::string CleanExtractedData(const std::string& data);
    std::map<std::string, std::string> ProcessExtractedData(const std::map<std::string, std::string>& raw_data);
    
    // File operations
    bool SaveScrapedData(const ScrapedPageData& data, const std::string& file_path);
    ScrapedPageData LoadScrapedData(const std::string& file_path);
    bool SaveSessionData(const ScrapingSession& session, const std::string& file_path);
    ScrapingSession LoadSessionData(const std::string& file_path);
}

} // namespace chromium_playwright
