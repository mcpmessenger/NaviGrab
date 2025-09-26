#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace chromium_playwright::network {

// HTTP Response structure
struct HTTPResponse {
    bool success = false;
    int status_code = 0;
    std::string body;
    std::map<std::string, std::string> headers;
    std::string error_message;
    double response_time_ms = 0.0;
    
    // Get header value
    std::string GetHeader(const std::string& name) const {
        auto it = headers.find(name);
        return (it != headers.end()) ? it->second : "";
    }
    
    // Check if response is successful
    bool IsSuccess() const {
        return success && status_code >= 200 && status_code < 300;
    }
    
    // Get content type
    std::string GetContentType() const {
        return GetHeader("Content-Type");
    }
    
    // Get content length
    size_t GetContentLength() const {
        std::string length_str = GetHeader("Content-Length");
        return length_str.empty() ? 0 : std::stoul(length_str);
    }
};

// HTTP Client interface
class HTTPClient {
public:
    virtual ~HTTPClient() = default;
    
    // Basic HTTP methods
    virtual HTTPResponse Get(const std::string& url) = 0;
    virtual HTTPResponse Post(const std::string& url, const std::string& body = "", 
                            const std::map<std::string, std::string>& headers = {}) = 0;
    virtual HTTPResponse Put(const std::string& url, const std::string& body = "", 
                           const std::map<std::string, std::string>& headers = {}) = 0;
    virtual HTTPResponse Delete(const std::string& url) = 0;
    
    // File operations
    virtual bool DownloadFile(const std::string& url, const std::string& file_path) = 0;
    
    // Advanced methods
    virtual HTTPResponse Head(const std::string& url) = 0;
    virtual HTTPResponse Options(const std::string& url) = 0;
    virtual HTTPResponse Patch(const std::string& url, const std::string& body = "", 
                             const std::map<std::string, std::string>& headers = {}) = 0;
    
    // Configuration
    virtual void SetTimeout(int timeout_ms) = 0;
    virtual void SetUserAgent(const std::string& user_agent) = 0;
    virtual void SetDefaultHeaders(const std::map<std::string, std::string>& headers) = 0;
    
    // Authentication
    virtual void SetBasicAuth(const std::string& username, const std::string& password) = 0;
    virtual void SetBearerToken(const std::string& token) = 0;
    
    // SSL/TLS
    virtual void SetVerifySSL(bool verify) = 0;
    virtual void SetCertFile(const std::string& cert_file) = 0;
    virtual void SetKeyFile(const std::string& key_file) = 0;
};

// Factory function
std::unique_ptr<HTTPClient> CreateHTTPClient();

// Utility functions
namespace http_utils {
    // URL encoding/decoding
    std::string EncodeURL(const std::string& url);
    std::string DecodeURL(const std::string& url);
    
    // Query parameters
    std::map<std::string, std::string> ParseQueryParams(const std::string& query_string);
    std::string BuildQueryString(const std::map<std::string, std::string>& params);
    
    // URL validation
    bool IsValidURL(const std::string& url);
    std::string ExtractDomain(const std::string& url);
    std::string ExtractPath(const std::string& url);
    std::string ExtractQuery(const std::string& url);
    
    // Content type helpers
    bool IsHTML(const std::string& content_type);
    bool IsJSON(const std::string& content_type);
    bool IsXML(const std::string& content_type);
    bool IsImage(const std::string& content_type);
    bool IsText(const std::string& content_type);
    
    // Response helpers
    std::string ExtractTitle(const std::string& html);
    std::vector<std::string> ExtractLinks(const std::string& html);
    std::vector<std::string> ExtractImages(const std::string& html);
    
    // Headers helpers
    std::string GetRedirectLocation(const HTTPResponse& response);
    bool IsRedirect(const HTTPResponse& response);
    std::string GetCharset(const std::string& content_type);
    
    // Error helpers
    std::string GetStatusMessage(int status_code);
    bool IsClientError(int status_code);
    bool IsServerError(int status_code);
    bool IsRetryableError(int status_code);
}

} // namespace chromium_playwright::network