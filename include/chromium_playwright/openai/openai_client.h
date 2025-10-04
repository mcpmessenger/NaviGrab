#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <future>
#include <nlohmann/json.hpp>

namespace chromium_playwright {
namespace openai {

// OpenAI API Configuration
struct OpenAIConfig {
    std::string api_key;
    std::string base_url = "https://api.openai.com/v1";
    std::string model = "gpt-4-vision-preview";
    int max_tokens = 1000;
    double temperature = 0.7;
    std::map<std::string, std::string> headers;
    
    OpenAIConfig() {
        headers["Content-Type"] = "application/json";
        headers["User-Agent"] = "NaviGrab/1.0.0";
    }
};

// Image input for Vision API
struct VisionImage {
    std::string type = "image_url";
    std::string image_url;
    std::string detail = "auto"; // "low", "high", "auto"
    
    VisionImage(const std::string& url) : image_url(url) {}
};

// Message for Vision API
struct VisionMessage {
    std::string role = "user";
    std::vector<std::string> content;
    std::vector<VisionImage> images;
    
    void AddText(const std::string& text) {
        content.push_back(text);
    }
    
    void AddImage(const std::string& image_url) {
        images.emplace_back(image_url);
    }
};

// Vision API Request
struct VisionRequest {
    std::string model = "gpt-4-vision-preview";
    std::vector<VisionMessage> messages;
    int max_tokens = 1000;
    double temperature = 0.7;
    bool stream = false;
    
    // Helper methods
    void AddMessage(const VisionMessage& message) {
        messages.push_back(message);
    }
    
    void AddUserMessage(const std::string& text, const std::vector<std::string>& image_urls = {}) {
        VisionMessage msg;
        msg.role = "user";
        msg.AddText(text);
        for (const auto& url : image_urls) {
            msg.AddImage(url);
        }
        messages.push_back(msg);
    }
};

// Vision API Response
struct VisionResponse {
    bool success = false;
    std::string error_message;
    std::string id;
    std::string object;
    std::chrono::system_clock::time_point created;
    std::string model;
    std::vector<Choice> choices;
    Usage usage;
    
    struct Choice {
        int index;
        VisionMessage message;
        std::string finish_reason;
    };
    
    struct Usage {
        int prompt_tokens;
        int completion_tokens;
        int total_tokens;
    };
    
    // Helper methods
    std::string GetContent() const {
        if (!choices.empty() && !choices[0].message.content.empty()) {
            return choices[0].message.content[0];
        }
        return "";
    }
    
    bool HasError() const {
        return !success || !error_message.empty();
    }
};

// Screenshot Analysis Request
struct ScreenshotAnalysisRequest {
    std::string screenshot_base64; // Base64 encoded screenshot
    std::string analysis_prompt = "Analyze this screenshot and provide detailed insights about the content, layout, and any notable elements.";
    std::string detail_level = "auto"; // "low", "high", "auto"
    int max_tokens = 1500;
    double temperature = 0.3;
    
    // Predefined analysis types
    enum class AnalysisType {
        GENERAL,
        UI_ANALYSIS,
        CONTENT_ANALYSIS,
        ACCESSIBILITY,
        PERFORMANCE,
        CUSTOM
    };
    
    AnalysisType type = AnalysisType::GENERAL;
    
    // Get prompt based on analysis type
    std::string GetPrompt() const {
        switch (type) {
            case AnalysisType::UI_ANALYSIS:
                return "Analyze this screenshot from a UI/UX perspective. Describe the layout, visual hierarchy, color scheme, typography, and overall design elements. Identify any usability issues or design patterns.";
            case AnalysisType::CONTENT_ANALYSIS:
                return "Analyze the content of this screenshot. Identify the main topics, key information, text content, images, and overall message being conveyed.";
            case AnalysisType::ACCESSIBILITY:
                return "Analyze this screenshot for accessibility considerations. Look for contrast issues, text readability, navigation elements, and overall accessibility compliance.";
            case AnalysisType::PERFORMANCE:
                return "Analyze this screenshot for performance-related visual cues. Look for loading states, error messages, slow-loading content, or performance indicators.";
            case AnalysisType::CUSTOM:
                return analysis_prompt;
            default: // GENERAL
                return analysis_prompt;
        }
    }
};

// Screenshot Analysis Response
struct ScreenshotAnalysisResponse {
    bool success = false;
    std::string error_message;
    std::string analysis_text;
    std::map<std::string, std::string> structured_data;
    std::vector<std::string> key_findings;
    std::vector<std::string> recommendations;
    std::chrono::system_clock::time_point timestamp;
    ScreenshotAnalysisRequest::AnalysisType analysis_type;
    
    // Helper methods
    void AddFinding(const std::string& finding) {
        key_findings.push_back(finding);
    }
    
    void AddRecommendation(const std::string& recommendation) {
        recommendations.push_back(recommendation);
    }
    
    void SetStructuredData(const std::string& key, const std::string& value) {
        structured_data[key] = value;
    }
    
    bool HasFindings() const {
        return !key_findings.empty();
    }
    
    bool HasRecommendations() const {
        return !recommendations.empty();
    }
};

// OpenAI Client Interface
class OpenAIClient {
public:
    virtual ~OpenAIClient() = default;
    
    // Configuration
    virtual void SetConfig(const OpenAIConfig& config) = 0;
    virtual OpenAIConfig GetConfig() const = 0;
    
    // Vision API
    virtual std::future<VisionResponse> AnalyzeImageAsync(const VisionRequest& request) = 0;
    virtual VisionResponse AnalyzeImage(const VisionRequest& request) = 0;
    
    // Screenshot Analysis (convenience methods)
    virtual std::future<ScreenshotAnalysisResponse> AnalyzeScreenshotAsync(const ScreenshotAnalysisRequest& request) = 0;
    virtual ScreenshotAnalysisResponse AnalyzeScreenshot(const ScreenshotAnalysisRequest& request) = 0;
    
    // Batch Analysis
    virtual std::future<std::vector<ScreenshotAnalysisResponse>> AnalyzeScreenshotsAsync(
        const std::vector<ScreenshotAnalysisRequest>& requests) = 0;
    virtual std::vector<ScreenshotAnalysisResponse> AnalyzeScreenshots(
        const std::vector<ScreenshotAnalysisRequest>& requests) = 0;
    
    // Utility methods
    virtual bool IsConfigured() const = 0;
    virtual std::string GetLastError() const = 0;
    virtual void ClearError() = 0;
    
    // Health check
    virtual bool TestConnection() = 0;
};

// Factory function
std::unique_ptr<OpenAIClient> CreateOpenAIClient();

} // namespace openai
} // namespace chromium_playwright

