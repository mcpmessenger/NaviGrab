#include "chromium_playwright/tooltip_system.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <json/json.h>

namespace chromium_playwright {

// Local file-based storage implementation
class FileTooltipStorage : public TooltipStorage {
private:
    std::string storage_path_;
    std::string data_file_path_;
    
public:
    FileTooltipStorage(const std::string& storage_path = "tooltip_data") 
        : storage_path_(storage_path), data_file_path_(storage_path + "/tooltip_data.json") {
        // Create storage directory if it doesn't exist
        std::filesystem::create_directories(storage_path_);
    }
    
    bool StoreTooltipData(const TooltipData& data) override {
        try {
            // Load existing data
            std::vector<TooltipData> all_data = GetAllTooltipData();
            
            // Remove existing entry with same URL
            all_data.erase(
                std::remove_if(all_data.begin(), all_data.end(),
                    [&data](const TooltipData& item) { return item.url == data.url; }),
                all_data.end()
            );
            
            // Add new data
            all_data.push_back(data);
            
            // Save to file
            return SaveToFile(all_data);
        } catch (const std::exception& e) {
            std::cerr << "Error storing tooltip data: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::unique_ptr<TooltipData> GetTooltipData(const std::string& url) override {
        try {
            std::vector<TooltipData> all_data = GetAllTooltipData();
            
            for (const auto& data : all_data) {
                if (data.url == url) {
                    return std::make_unique<TooltipData>(data);
                }
            }
            return nullptr;
        } catch (const std::exception& e) {
            std::cerr << "Error getting tooltip data: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    std::vector<TooltipData> GetAllTooltipData() override {
        std::vector<TooltipData> data;
        
        if (!std::filesystem::exists(data_file_path_)) {
            return data;
        }
        
        try {
            std::ifstream file(data_file_path_);
            if (!file.is_open()) {
                return data;
            }
            
            Json::Value root;
            file >> root;
            
            if (root.isArray()) {
                for (const auto& item : root) {
                    TooltipData tooltip_data;
                    tooltip_data.id = item.get("id", "").asString();
                    tooltip_data.url = item.get("url", "").asString();
                    tooltip_data.title = item.get("title", "").asString();
                    tooltip_data.base64_screenshot = item.get("base64_screenshot", "").asString();
                    tooltip_data.description = item.get("description", "").asString();
                    tooltip_data.links_found = item.get("links_found", 0).asInt();
                    tooltip_data.buttons_found = item.get("buttons_found", 0).asInt();
                    tooltip_data.depth = item.get("depth", 0).asInt();
                    
                    // Parse timestamp
                    std::string timestamp_str = item.get("timestamp", "").asString();
                    if (!timestamp_str.empty()) {
                        std::istringstream iss(timestamp_str);
                        std::chrono::system_clock::time_point timestamp;
                        iss >> timestamp;
                        tooltip_data.timestamp = timestamp;
                    }
                    
                    data.push_back(tooltip_data);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading tooltip data: " << e.what() << std::endl;
        }
        
        return data;
    }
    
    bool ClearAllData() override {
        try {
            if (std::filesystem::exists(data_file_path_)) {
                std::filesystem::remove(data_file_path_);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error clearing tooltip data: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool HasDataForUrl(const std::string& url) override {
        return GetTooltipData(url) != nullptr;
    }
    
private:
    bool SaveToFile(const std::vector<TooltipData>& data) {
        try {
            Json::Value root(Json::arrayValue);
            
            for (const auto& item : data) {
                Json::Value item_json;
                item_json["id"] = item.id;
                item_json["url"] = item.url;
                item_json["title"] = item.title;
                item_json["base64_screenshot"] = item.base64_screenshot;
                item_json["description"] = item.description;
                item_json["links_found"] = item.links_found;
                item_json["buttons_found"] = item.buttons_found;
                item_json["depth"] = item.depth;
                
                // Convert timestamp to string
                std::ostringstream oss;
                oss << item.timestamp;
                item_json["timestamp"] = oss.str();
                
                root.append(item_json);
            }
            
            std::ofstream file(data_file_path_);
            if (!file.is_open()) {
                return false;
            }
            
            file << root;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving tooltip data: " << e.what() << std::endl;
            return false;
        }
    }
};

// Chrome extension storage implementation (using Chrome's storage API)
class ChromeExtensionTooltipStorage : public TooltipStorage {
private:
    // This would integrate with Chrome's chrome.storage.local API
    // For now, we'll use a simple in-memory storage
    std::map<std::string, TooltipData> storage_;
    
public:
    bool StoreTooltipData(const TooltipData& data) override {
        storage_[data.url] = data;
        return true;
    }
    
    std::unique_ptr<TooltipData> GetTooltipData(const std::string& url) override {
        auto it = storage_.find(url);
        if (it != storage_.end()) {
            return std::make_unique<TooltipData>(it->second);
        }
        return nullptr;
    }
    
    std::vector<TooltipData> GetAllTooltipData() override {
        std::vector<TooltipData> data;
        for (const auto& pair : storage_) {
            data.push_back(pair.second);
        }
        return data;
    }
    
    bool ClearAllData() override {
        storage_.clear();
        return true;
    }
    
    bool HasDataForUrl(const std::string& url) override {
        return storage_.find(url) != storage_.end();
    }
};

// Factory function
std::unique_ptr<TooltipStorage> CreateTooltipStorage() {
    // In a Chrome fork, this would return ChromeExtensionTooltipStorage
    // For now, return file-based storage
    return std::make_unique<FileTooltipStorage>();
}

} // namespace chromium_playwright


