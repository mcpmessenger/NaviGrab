#include "chromium_playwright/openai/screenshot_analyzer.h"
#include "chromium_playwright/screenshot_capture.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace chromium_playwright {
namespace openai {

// Screenshot Analyzer Implementation
class ScreenshotAnalyzerImpl : public ScreenshotAnalyzer {
private:
    std::unique_ptr<OpenAIClient> openai_client_;
    std::unique_ptr<screenshot_capture::ScreenshotCapture> screenshot_capture_;
    AnalysisSettings settings_;
    std::function<void(const std::string&, int)> progress_callback_;
    
public:
    ScreenshotAnalyzerImpl() {
        openai_client_ = CreateOpenAIClient();
        screenshot_capture_ = screenshot_capture::CreateScreenshotCapture();
        settings_ = AnalysisSettings();
    }
    
    virtual ~ScreenshotAnalyzerImpl() = default;
    
    void SetOpenAIConfig(const OpenAIConfig& config) override {
        if (openai_client_) {
            openai_client_->SetConfig(config);
        }
    }
    
    void SetAnalysisSettings(const AnalysisSettings& settings) override {
        settings_ = settings;
    }
    
    std::future<ScreenshotAnalysisResponse> AnalyzeScreenshotAsync(
        const std::vector<uint8_t>& screenshot_data,
        ScreenshotAnalysisRequest::AnalysisType type,
        const std::string& custom_prompt) override {
        return std::async(std::launch::async, [this, screenshot_data, type, custom_prompt]() {
            return AnalyzeScreenshot(screenshot_data, type, custom_prompt);
        });
    }
    
    ScreenshotAnalysisResponse AnalyzeScreenshot(
        const std::vector<uint8_t>& screenshot_data,
        ScreenshotAnalysisRequest::AnalysisType type,
        const std::string& custom_prompt) override {
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (progress_callback_) {
            progress_callback_("Converting screenshot to base64...", 10);
        }
        
        try {
            if (!openai_client_ || !openai_client_->IsConfigured()) {
                ScreenshotAnalysisResponse error_response;
                error_response.success = false;
                error_response.error_message = "OpenAI client not configured";
                return error_response;
            }
            
            // Convert screenshot data to base64
            std::string base64_data = ConvertToBase64(screenshot_data);
            
            if (progress_callback_) {
                progress_callback_("Preparing analysis request...", 30);
            }
            
            // Create analysis request
            ScreenshotAnalysisRequest request;
            request.screenshot_base64 = base64_data;
            request.type = type;
            request.max_tokens = settings_.max_tokens;
            request.temperature = settings_.temperature;
            request.detail_level = settings_.detail_level;
            
            if (!custom_prompt.empty()) {
                request.analysis_prompt = custom_prompt;
                request.type = ScreenshotAnalysisRequest::AnalysisType::CUSTOM;
            }
            
            if (progress_callback_) {
                progress_callback_("Sending request to OpenAI...", 50);
            }
            
            // Call OpenAI API
            auto response = openai_client_->AnalyzeScreenshot(request);
            
            if (progress_callback_) {
                progress_callback_("Processing response...", 80);
            }
            
            // Enhance the response with additional processing
            if (response.success && settings_.extract_structured_data) {
                response = EnhanceAnalysisResponse(response, screenshot_data);
            }
            
            if (progress_callback_) {
                progress_callback_("Analysis complete!", 100);
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            std::cout << "✅ Screenshot analysis completed in " << duration.count() << "ms" << std::endl;
            
            return response;
            
        } catch (const std::exception& e) {
            ScreenshotAnalysisResponse error_response;
            error_response.success = false;
            error_response.error_message = "Exception: " + std::string(e.what());
            std::cout << "❌ Screenshot analysis failed: " << error_response.error_message << std::endl;
            return error_response;
        }
    }
    
    std::future<std::vector<ScreenshotAnalysisResponse>> AnalyzeScreenshotsAsync(
        const std::vector<std::vector<uint8_t>>& screenshots_data,
        ScreenshotAnalysisRequest::AnalysisType type) override {
        return std::async(std::launch::async, [this, screenshots_data, type]() {
            return AnalyzeScreenshots(screenshots_data, type);
        });
    }
    
    std::vector<ScreenshotAnalysisResponse> AnalyzeScreenshots(
        const std::vector<std::vector<uint8_t>>& screenshots_data,
        ScreenshotAnalysisRequest::AnalysisType type) override {
        
        std::vector<ScreenshotAnalysisResponse> responses;
        
        for (size_t i = 0; i < screenshots_data.size(); ++i) {
            if (progress_callback_) {
                progress_callback_("Analyzing screenshot " + std::to_string(i + 1) + 
                                 " of " + std::to_string(screenshots_data.size()), 
                                 static_cast<int>((i * 100) / screenshots_data.size()));
            }
            
            auto response = AnalyzeScreenshot(screenshots_data[i], type);
            responses.push_back(response);
            
            // Small delay between requests
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        return responses;
    }
    
    std::future<ScreenshotAnalysisResponse> CaptureAndAnalyzeAsync(
        const std::string& url,
        ScreenshotAnalysisRequest::AnalysisType type,
        const std::string& custom_prompt) override {
        return std::async(std::launch::async, [this, url, type, custom_prompt]() {
            return CaptureAndAnalyze(url, type, custom_prompt);
        });
    }
    
    ScreenshotAnalysisResponse CaptureAndAnalyze(
        const std::string& url,
        ScreenshotAnalysisRequest::AnalysisType type,
        const std::string& custom_prompt) override {
        
        if (progress_callback_) {
            progress_callback_("Capturing screenshot of " + url + "...", 5);
        }
        
        try {
            if (!screenshot_capture_) {
                ScreenshotAnalysisResponse error_response;
                error_response.success = false;
                error_response.error_message = "Screenshot capture not available";
                return error_response;
            }
            
            // Create a mock page for screenshot capture
            // In real implementation, this would be a real browser page
            screenshot_capture::Page mock_page;
            screenshot_capture::ScreenshotOptions options;
            options.format = screenshot_capture::ImageFormat::PNG;
            options.full_page = true;
            options.quality = 90;
            
            // Capture screenshot
            auto screenshot_result = screenshot_capture_->CapturePage(mock_page, options);
            
            if (!screenshot_result.success) {
                ScreenshotAnalysisResponse error_response;
                error_response.success = false;
                error_response.error_message = "Screenshot capture failed: " + screenshot_result.error_message;
                return error_response;
            }
            
            if (progress_callback_) {
                progress_callback_("Screenshot captured, starting analysis...", 20);
            }
            
            // Analyze the captured screenshot
            auto analysis_response = AnalyzeScreenshot(screenshot_result.image_data, type, custom_prompt);
            
            // Add capture metadata
            analysis_response.metadata["url"] = url;
            analysis_response.metadata["capture_time"] = std::to_string(
                std::chrono::duration_cast<std::chrono::seconds>(
                    screenshot_result.metadata.timestamp.time_since_epoch()).count());
            analysis_response.metadata["width"] = std::to_string(screenshot_result.metadata.width);
            analysis_response.metadata["height"] = std::to_string(screenshot_result.metadata.height);
            
            return analysis_response;
            
        } catch (const std::exception& e) {
            ScreenshotAnalysisResponse error_response;
            error_response.success = false;
            error_response.error_message = "Exception: " + std::string(e.what());
            std::cout << "❌ Capture and analyze failed: " << error_response.error_message << std::endl;
            return error_response;
        }
    }
    
    bool IsConfigured() const override {
        return openai_client_ && openai_client_->IsConfigured();
    }
    
    std::string GetLastError() const override {
        return openai_client_ ? openai_client_->GetLastError() : "No OpenAI client available";
    }
    
    void SetProgressCallback(std::function<void(const std::string&, int)> callback) override {
        progress_callback_ = callback;
    }
    
private:
    std::string ConvertToBase64(const std::vector<uint8_t>& data) {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (uint8_t c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (result.size() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
    
    ScreenshotAnalysisResponse EnhanceAnalysisResponse(
        const ScreenshotAnalysisResponse& response,
        const std::vector<uint8_t>& screenshot_data) {
        
        ScreenshotAnalysisResponse enhanced = response;
        
        // Add performance metrics
        enhanced.metadata["input_size_bytes"] = std::to_string(screenshot_data.size());
        
        // Extract additional structured data
        std::string lower_text = enhanced.analysis_text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
        
        // Look for specific patterns and add findings
        if (lower_text.find("button") != std::string::npos) {
            enhanced.AddFinding("Interactive buttons detected in the interface");
        }
        
        if (lower_text.find("form") != std::string::npos) {
            enhanced.AddFinding("Form elements present on the page");
        }
        
        if (lower_text.find("navigation") != std::string::npos) {
            enhanced.AddFinding("Navigation elements identified");
        }
        
        if (lower_text.find("error") != std::string::npos) {
            enhanced.AddFinding("Error messages or issues detected");
            enhanced.AddRecommendation("Review error messages and fix any issues");
        }
        
        if (lower_text.find("loading") != std::string::npos) {
            enhanced.AddFinding("Loading states or progress indicators present");
        }
        
        // Add UI/UX recommendations based on analysis
        if (lower_text.find("cluttered") != std::string::npos || 
            lower_text.find("busy") != std::string::npos) {
            enhanced.AddRecommendation("Consider simplifying the layout to reduce visual clutter");
        }
        
        if (lower_text.find("small") != std::string::npos && 
            lower_text.find("text") != std::string::npos) {
            enhanced.AddRecommendation("Consider increasing text size for better readability");
        }
        
        return enhanced;
    }
};

// Factory function implementation
std::unique_ptr<ScreenshotAnalyzer> CreateScreenshotAnalyzer() {
    return std::make_unique<ScreenshotAnalyzerImpl>();
}

} // namespace openai
} // namespace chromium_playwright

