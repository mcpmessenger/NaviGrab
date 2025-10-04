#include "chromium_playwright/openai/openai_client.h"
#include "chromium_playwright/network/http_client.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <future>

namespace chromium_playwright {
namespace openai {

// OpenAI Client Implementation
class OpenAIClientImpl : public OpenAIClient {
private:
    OpenAIConfig config_;
    std::string last_error_;
    std::unique_ptr<network::HttpClient> http_client_;
    
public:
    OpenAIClientImpl() {
        http_client_ = network::CreateHttpClient();
        config_.headers["Content-Type"] = "application/json";
        config_.headers["User-Agent"] = "NaviGrab/1.0.0";
    }
    
    virtual ~OpenAIClientImpl() = default;
    
    void SetConfig(const OpenAIConfig& config) override {
        config_ = config;
        if (!config_.api_key.empty()) {
            config_.headers["Authorization"] = "Bearer " + config_.api_key;
        }
    }
    
    OpenAIConfig GetConfig() const override {
        return config_;
    }
    
    std::future<VisionResponse> AnalyzeImageAsync(const VisionRequest& request) override {
        return std::async(std::launch::async, [this, request]() {
            return AnalyzeImage(request);
        });
    }
    
    VisionResponse AnalyzeImage(const VisionRequest& request) override {
        VisionResponse response;
        
        try {
            if (!IsConfigured()) {
                response.success = false;
                response.error_message = "OpenAI client not configured. Please set API key.";
                return response;
            }
            
            // Prepare the request JSON
            nlohmann::json request_json;
            request_json["model"] = request.model;
            request_json["max_tokens"] = request.max_tokens;
            request_json["temperature"] = request.temperature;
            request_json["stream"] = request.stream;
            
            // Build messages array
            nlohmann::json messages_array = nlohmann::json::array();
            for (const auto& msg : request.messages) {
                nlohmann::json message_json;
                message_json["role"] = msg.role;
                
                nlohmann::json content_array = nlohmann::json::array();
                
                // Add text content
                for (const auto& text : msg.content) {
                    nlohmann::json text_obj;
                    text_obj["type"] = "text";
                    text_obj["text"] = text;
                    content_array.push_back(text_obj);
                }
                
                // Add image content
                for (const auto& img : msg.images) {
                    nlohmann::json image_obj;
                    image_obj["type"] = "image_url";
                    nlohmann::json image_url_obj;
                    image_url_obj["url"] = img.image_url;
                    image_url_obj["detail"] = img.detail;
                    image_obj["image_url"] = image_url_obj;
                    content_array.push_back(image_obj);
                }
                
                message_json["content"] = content_array;
                messages_array.push_back(message_json);
            }
            
            request_json["messages"] = messages_array;
            
            // Make HTTP request
            std::string request_body = request_json.dump();
            std::string url = config_.base_url + "/chat/completions";
            
            auto http_response = http_client_->Post(url, request_body, config_.headers);
            
            if (!http_response.success) {
                response.success = false;
                response.error_message = "HTTP request failed: " + http_response.error_message;
                last_error_ = response.error_message;
                return response;
            }
            
            // Parse response
            nlohmann::json response_json;
            try {
                response_json = nlohmann::json::parse(http_response.body);
            } catch (const nlohmann::json::exception& e) {
                response.success = false;
                response.error_message = "Failed to parse JSON response: " + std::string(e.what());
                last_error_ = response.error_message;
                return response;
            }
            
            // Check for API errors
            if (response_json.contains("error")) {
                response.success = false;
                response.error_message = response_json["error"]["message"];
                last_error_ = response.error_message;
                return response;
            }
            
            // Parse successful response
            response.success = true;
            response.id = response_json.value("id", "");
            response.object = response_json.value("object", "");
            response.model = response_json.value("model", "");
            
            if (response_json.contains("created")) {
                auto created_timestamp = response_json["created"].get<long>();
                response.created = std::chrono::system_clock::from_time_t(created_timestamp);
            }
            
            // Parse choices
            if (response_json.contains("choices")) {
                for (const auto& choice_json : response_json["choices"]) {
                    VisionResponse::Choice choice;
                    choice.index = choice_json.value("index", 0);
                    choice.finish_reason = choice_json.value("finish_reason", "");
                    
                    if (choice_json.contains("message")) {
                        const auto& message_json = choice_json["message"];
                        choice.message.role = message_json.value("role", "");
                        
                        if (message_json.contains("content")) {
                            if (message_json["content"].is_array()) {
                                for (const auto& content_item : message_json["content"]) {
                                    if (content_item.contains("text")) {
                                        choice.message.content.push_back(content_item["text"]);
                                    }
                                }
                            } else if (message_json["content"].is_string()) {
                                choice.message.content.push_back(message_json["content"]);
                            }
                        }
                    }
                    
                    response.choices.push_back(choice);
                }
            }
            
            // Parse usage
            if (response_json.contains("usage")) {
                const auto& usage_json = response_json["usage"];
                response.usage.prompt_tokens = usage_json.value("prompt_tokens", 0);
                response.usage.completion_tokens = usage_json.value("completion_tokens", 0);
                response.usage.total_tokens = usage_json.value("total_tokens", 0);
            }
            
            std::cout << "✅ OpenAI Vision API analysis completed successfully" << std::endl;
            std::cout << "   Tokens used: " << response.usage.total_tokens << std::endl;
            
        } catch (const std::exception& e) {
            response.success = false;
            response.error_message = "Exception: " + std::string(e.what());
            last_error_ = response.error_message;
            std::cout << "❌ OpenAI analysis failed: " << response.error_message << std::endl;
        }
        
        return response;
    }
    
    std::future<ScreenshotAnalysisResponse> AnalyzeScreenshotAsync(const ScreenshotAnalysisRequest& request) override {
        return std::async(std::launch::async, [this, request]() {
            return AnalyzeScreenshot(request);
        });
    }
    
    ScreenshotAnalysisResponse AnalyzeScreenshot(const ScreenshotAnalysisRequest& request) override {
        ScreenshotAnalysisResponse response;
        response.timestamp = std::chrono::system_clock::now();
        response.analysis_type = request.type;
        
        try {
            // Convert base64 screenshot to data URL
            std::string image_url = "data:image/png;base64," + request.screenshot_base64;
            
            // Create Vision API request
            VisionRequest vision_request;
            vision_request.model = config_.model;
            vision_request.max_tokens = request.max_tokens;
            vision_request.temperature = request.temperature;
            
            // Add user message with image
            VisionMessage message;
            message.role = "user";
            message.AddText(request.GetPrompt());
            message.AddImage(image_url);
            vision_request.AddMessage(message);
            
            // Call Vision API
            auto vision_response = AnalyzeImage(vision_request);
            
            if (!vision_response.success) {
                response.success = false;
                response.error_message = vision_response.error_message;
                return response;
            }
            
            // Process the response
            response.success = true;
            response.analysis_text = vision_response.GetContent();
            
            // Extract structured data from the analysis
            ExtractStructuredData(response.analysis_text, response);
            
            std::cout << "✅ Screenshot analysis completed successfully" << std::endl;
            std::cout << "   Analysis type: " << static_cast<int>(request.type) << std::endl;
            std::cout << "   Text length: " << response.analysis_text.length() << " characters" << std::endl;
            
        } catch (const std::exception& e) {
            response.success = false;
            response.error_message = "Exception: " + std::string(e.what());
            std::cout << "❌ Screenshot analysis failed: " << response.error_message << std::endl;
        }
        
        return response;
    }
    
    std::future<std::vector<ScreenshotAnalysisResponse>> AnalyzeScreenshotsAsync(
        const std::vector<ScreenshotAnalysisRequest>& requests) override {
        return std::async(std::launch::async, [this, requests]() {
            return AnalyzeScreenshots(requests);
        });
    }
    
    std::vector<ScreenshotAnalysisResponse> AnalyzeScreenshots(
        const std::vector<ScreenshotAnalysisRequest>& requests) override {
        std::vector<ScreenshotAnalysisResponse> responses;
        
        for (const auto& request : requests) {
            auto response = AnalyzeScreenshot(request);
            responses.push_back(response);
            
            // Small delay between requests to avoid rate limiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        return responses;
    }
    
    bool IsConfigured() const override {
        return !config_.api_key.empty() && !config_.base_url.empty();
    }
    
    std::string GetLastError() const override {
        return last_error_;
    }
    
    void ClearError() override {
        last_error_.clear();
    }
    
    bool TestConnection() override {
        try {
            // Simple test request
            VisionRequest test_request;
            test_request.model = config_.model;
            test_request.max_tokens = 10;
            
            VisionMessage test_message;
            test_message.role = "user";
            test_message.AddText("Say 'test' if you can read this.");
            test_request.AddMessage(test_message);
            
            auto response = AnalyzeImage(test_request);
            return response.success;
            
        } catch (const std::exception& e) {
            last_error_ = "Connection test failed: " + std::string(e.what());
            return false;
        }
    }
    
private:
    void ExtractStructuredData(const std::string& analysis_text, ScreenshotAnalysisResponse& response) {
        // Simple keyword extraction for structured data
        std::string lower_text = analysis_text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
        
        // Look for common patterns and extract them
        if (lower_text.find("button") != std::string::npos) {
            response.SetStructuredData("has_buttons", "true");
            response.AddFinding("Interactive buttons detected");
        }
        
        if (lower_text.find("form") != std::string::npos) {
            response.SetStructuredData("has_forms", "true");
            response.AddFinding("Form elements detected");
        }
        
        if (lower_text.find("navigation") != std::string::npos) {
            response.SetStructuredData("has_navigation", "true");
            response.AddFinding("Navigation elements detected");
        }
        
        if (lower_text.find("error") != std::string::npos) {
            response.SetStructuredData("has_errors", "true");
            response.AddFinding("Error messages detected");
        }
        
        if (lower_text.find("loading") != std::string::npos) {
            response.SetStructuredData("has_loading", "true");
            response.AddFinding("Loading states detected");
        }
        
        // Extract recommendations from the text
        if (lower_text.find("recommend") != std::string::npos || 
            lower_text.find("suggest") != std::string::npos ||
            lower_text.find("improve") != std::string::npos) {
            response.AddRecommendation("Review analysis text for specific recommendations");
        }
    }
};

// Factory function implementation
std::unique_ptr<OpenAIClient> CreateOpenAIClient() {
    return std::make_unique<OpenAIClientImpl>();
}

} // namespace openai
} // namespace chromium_playwright

