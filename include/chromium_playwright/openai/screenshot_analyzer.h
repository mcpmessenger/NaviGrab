#pragma once

#include "openai_client.h"
#include "chromium_playwright/screenshot_capture.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>

namespace chromium_playwright {
namespace openai {

// Screenshot Analyzer Interface
class ScreenshotAnalyzer {
public:
    virtual ~ScreenshotAnalyzer() = default;
    
    // Configuration
    virtual void SetOpenAIConfig(const OpenAIConfig& config) = 0;
    virtual void SetAnalysisSettings(const AnalysisSettings& settings) = 0;
    
    // Analysis methods
    virtual std::future<ScreenshotAnalysisResponse> AnalyzeScreenshotAsync(
        const std::vector<uint8_t>& screenshot_data, 
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL,
        const std::string& custom_prompt = "") = 0;
    
    virtual ScreenshotAnalysisResponse AnalyzeScreenshot(
        const std::vector<uint8_t>& screenshot_data,
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL,
        const std::string& custom_prompt = "") = 0;
    
    // Batch analysis
    virtual std::future<std::vector<ScreenshotAnalysisResponse>> AnalyzeScreenshotsAsync(
        const std::vector<std::vector<uint8_t>>& screenshots_data,
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL) = 0;
    
    virtual std::vector<ScreenshotAnalysisResponse> AnalyzeScreenshots(
        const std::vector<std::vector<uint8_t>>& screenshots_data,
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL) = 0;
    
    // Integration with screenshot capture
    virtual std::future<ScreenshotAnalysisResponse> CaptureAndAnalyzeAsync(
        const std::string& url,
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL,
        const std::string& custom_prompt = "") = 0;
    
    virtual ScreenshotAnalysisResponse CaptureAndAnalyze(
        const std::string& url,
        ScreenshotAnalysisRequest::AnalysisType type = ScreenshotAnalysisRequest::AnalysisType::GENERAL,
        const std::string& custom_prompt = "") = 0;
    
    // Utility methods
    virtual bool IsConfigured() const = 0;
    virtual std::string GetLastError() const = 0;
    virtual void SetProgressCallback(std::function<void(const std::string&, int)> callback) = 0;
    
    // Analysis settings
    struct AnalysisSettings {
        int max_tokens = 1500;
        double temperature = 0.3;
        std::string detail_level = "auto";
        bool extract_structured_data = true;
        bool generate_recommendations = true;
        std::map<std::string, std::string> custom_headers;
        
        // Predefined analysis templates
        static AnalysisSettings GetUIAnalysisSettings() {
            AnalysisSettings settings;
            settings.max_tokens = 2000;
            settings.temperature = 0.2;
            settings.detail_level = "high";
            return settings;
        }
        
        static AnalysisSettings GetContentAnalysisSettings() {
            AnalysisSettings settings;
            settings.max_tokens = 1500;
            settings.temperature = 0.1;
            settings.detail_level = "high";
            return settings;
        }
        
        static AnalysisSettings GetQuickAnalysisSettings() {
            AnalysisSettings settings;
            settings.max_tokens = 500;
            settings.temperature = 0.5;
            settings.detail_level = "low";
            return settings;
        }
    };
};

// Enhanced Screenshot Analysis Response
struct EnhancedScreenshotAnalysisResponse : public ScreenshotAnalysisResponse {
    // Additional metadata
    std::string url;
    std::chrono::system_clock::time_point capture_time;
    std::map<std::string, std::string> metadata;
    
    // Analysis confidence scores
    struct ConfidenceScores {
        double overall_confidence = 0.0;
        double content_confidence = 0.0;
        double ui_confidence = 0.0;
        double accessibility_confidence = 0.0;
    } confidence;
    
    // Performance metrics
    struct PerformanceMetrics {
        std::chrono::milliseconds analysis_time;
        size_t input_size_bytes;
        int tokens_used;
        double cost_estimate;
    } performance;
    
    // Enhanced findings
    struct Finding {
        std::string category;
        std::string description;
        std::string severity; // "low", "medium", "high", "critical"
        std::vector<std::string> evidence;
        double confidence_score;
        
        Finding(const std::string& cat, const std::string& desc, const std::string& sev = "medium")
            : category(cat), description(desc), severity(sev), confidence_score(0.8) {}
    };
    
    struct Recommendation {
        std::string category;
        std::string description;
        std::string priority; // "low", "medium", "high", "urgent"
        std::vector<std::string> steps;
        
        Recommendation(const std::string& cat, const std::string& desc, const std::string& pri = "medium")
            : category(cat), description(desc), priority(pri) {}
    };
    
    std::vector<Finding> enhanced_findings;
    std::vector<Recommendation> enhanced_recommendations;
    
    // Helper methods
    void AddFinding(const Finding& finding) {
        enhanced_findings.push_back(finding);
        key_findings.push_back(finding.description);
    }
    
    void AddRecommendation(const Recommendation& recommendation) {
        enhanced_recommendations.push_back(recommendation);
        recommendations.push_back(recommendation.description);
    }
    
    std::vector<Finding> GetFindingsByCategory(const std::string& category) const {
        std::vector<Finding> result;
        for (const auto& finding : enhanced_findings) {
            if (finding.category == category) {
                result.push_back(finding);
            }
        }
        return result;
    }
    
    std::vector<Finding> GetFindingsBySeverity(const std::string& severity) const {
        std::vector<Finding> result;
        for (const auto& finding : enhanced_findings) {
            if (finding.severity == severity) {
                result.push_back(finding);
            }
        }
        return result;
    }
    
    std::vector<Recommendation> GetRecommendationsByPriority(const std::string& priority) const {
        std::vector<Recommendation> result;
        for (const auto& rec : enhanced_recommendations) {
            if (rec.priority == priority) {
                result.push_back(rec);
            }
        }
        return result;
    }
    
    bool HasHighSeverityIssues() const {
        return !GetFindingsBySeverity("high").empty() || !GetFindingsBySeverity("critical").empty();
    }
    
    bool HasUrgentRecommendations() const {
        return !GetRecommendationsByPriority("urgent").empty();
    }
};

// Factory function
std::unique_ptr<ScreenshotAnalyzer> CreateScreenshotAnalyzer();

} // namespace openai
} // namespace chromium_playwright

