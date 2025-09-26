#include "http_client.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

namespace chromium_playwright::network {

// HTTP Client Implementation
class HTTPClientImpl : public HTTPClient {
public:
    HTTPClientImpl() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    
    ~HTTPClientImpl() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    HTTPResponse Get(const std::string& url) override {
        return MakeRequest("GET", url, "", {});
    }
    
    HTTPResponse Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) override {
        return MakeRequest("POST", url, body, headers);
    }
    
    HTTPResponse Put(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) override {
        return MakeRequest("PUT", url, body, headers);
    }
    
    HTTPResponse Delete(const std::string& url) override {
        return MakeRequest("DELETE", url, "", {});
    }
    
    bool DownloadFile(const std::string& url, const std::string& file_path) override {
        HTTPResponse response = Get(url);
        if (response.success && !response.body.empty()) {
            std::ofstream file(file_path, std::ios::binary);
            if (file.is_open()) {
                file.write(response.body.c_str(), response.body.size());
                file.close();
                return true;
            }
        }
        return false;
    }

private:
    struct URLParts {
        std::string protocol;
        std::string host;
        int port;
        std::string path;
    };
    
    URLParts ParseURL(const std::string& url) {
        URLParts parts;
        
        // Simple URL parsing
        std::regex url_regex(R"(^(https?)://([^:/]+)(?::(\d+))?(/.*)?$)");
        std::smatch matches;
        
        if (std::regex_match(url, matches, url_regex)) {
            parts.protocol = matches[1].str();
            parts.host = matches[2].str();
            parts.port = matches[3].str().empty() ? 
                        (parts.protocol == "https" ? 443 : 80) : 
                        std::stoi(matches[3].str());
            parts.path = matches[4].str().empty() ? "/" : matches[4].str();
        }
        
        return parts;
    }
    
    HTTPResponse MakeRequest(const std::string& method, const std::string& url, 
                           const std::string& body, const std::map<std::string, std::string>& headers) {
        HTTPResponse response;
        
        try {
            URLParts url_parts = ParseURL(url);
            if (url_parts.host.empty()) {
                response.success = false;
                response.error_message = "Invalid URL";
                return response;
            }
            
            // Create socket
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                response.success = false;
                response.error_message = "Failed to create socket";
                return response;
            }
            
            // Resolve hostname
            struct hostent* host_entry = gethostbyname(url_parts.host.c_str());
            if (!host_entry) {
                response.success = false;
                response.error_message = "Failed to resolve hostname";
                close(sock);
                return response;
            }
            
            // Set up address
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(url_parts.port);
            server_addr.sin_addr = *((struct in_addr*)host_entry->h_addr);
            
            // Connect
            if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                response.success = false;
                response.error_message = "Failed to connect to server";
                close(sock);
                return response;
            }
            
            // Build HTTP request
            std::string request = BuildHTTPRequest(method, url_parts, body, headers);
            
            // Send request
            if (send(sock, request.c_str(), request.length(), 0) < 0) {
                response.success = false;
                response.error_message = "Failed to send request";
                close(sock);
                return response;
            }
            
            // Receive response
            response = ReceiveHTTPResponse(sock);
            
            close(sock);
            
        } catch (const std::exception& e) {
            response.success = false;
            response.error_message = e.what();
        }
        
        return response;
    }
    
    std::string BuildHTTPRequest(const std::string& method, const URLParts& url_parts, 
                               const std::string& body, const std::map<std::string, std::string>& headers) {
        std::ostringstream request;
        
        // Request line
        request << method << " " << url_parts.path << " HTTP/1.1\r\n";
        
        // Headers
        request << "Host: " << url_parts.host;
        if (url_parts.port != (url_parts.protocol == "https" ? 443 : 80)) {
            request << ":" << url_parts.port;
        }
        request << "\r\n";
        
        request << "User-Agent: ChromiumPlaywright/1.0\r\n";
        request << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
        request << "Accept-Language: en-US,en;q=0.5\r\n";
        request << "Accept-Encoding: identity\r\n";
        request << "Connection: close\r\n";
        
        // Custom headers
        for (const auto& header : headers) {
            request << header.first << ": " << header.second << "\r\n";
        }
        
        // Content-Length for POST/PUT
        if (!body.empty()) {
            request << "Content-Length: " << body.length() << "\r\n";
        }
        
        request << "\r\n";
        
        // Body
        if (!body.empty()) {
            request << body;
        }
        
        return request.str();
    }
    
    HTTPResponse ReceiveHTTPResponse(int sock) {
        HTTPResponse response;
        
        char buffer[4096];
        std::string raw_response;
        
        // Receive data
        int bytes_received;
        while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            raw_response += buffer;
        }
        
        if (bytes_received < 0) {
            response.success = false;
            response.error_message = "Failed to receive response";
            return response;
        }
        
        // Parse response
        size_t header_end = raw_response.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            response.success = false;
            response.error_message = "Invalid HTTP response";
            return response;
        }
        
        std::string headers_str = raw_response.substr(0, header_end);
        response.body = raw_response.substr(header_end + 4);
        
        // Parse status line
        size_t first_line_end = headers_str.find("\r\n");
        if (first_line_end == std::string::npos) {
            response.success = false;
            response.error_message = "Invalid HTTP response";
            return response;
        }
        
        std::string status_line = headers_str.substr(0, first_line_end);
        std::istringstream status_stream(status_line);
        std::string http_version, status_code_str, status_message;
        status_stream >> http_version >> status_code_str >> status_message;
        
        response.status_code = std::stoi(status_code_str);
        response.success = (response.status_code >= 200 && response.status_code < 300);
        
        // Parse headers
        std::string remaining_headers = headers_str.substr(first_line_end + 2);
        std::istringstream headers_stream(remaining_headers);
        std::string line;
        
        while (std::getline(headers_stream, line)) {
            if (line.empty() || line == "\r") continue;
            
            size_t colon_pos = line.find(": ");
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 2);
                
                // Remove trailing \r
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                
                response.headers[key] = value;
            }
        }
        
        return response;
    }
};

// Factory function
std::unique_ptr<HTTPClient> CreateHTTPClient() {
    return std::make_unique<HTTPClientImpl>();
}

// Utility functions
namespace http_utils {
    // URL encoding
    std::string EncodeURL(const std::string& url) {
        std::ostringstream encoded;
        encoded.fill('0');
        encoded << std::hex;
        
        for (char c : url) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded << c;
            } else {
                encoded << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
            }
        }
        
        return encoded.str();
    }
    
    // URL decoding
    std::string DecodeURL(const std::string& url) {
        std::ostringstream decoded;
        
        for (size_t i = 0; i < url.length(); ++i) {
            if (url[i] == '%' && i + 2 < url.length()) {
                std::string hex = url.substr(i + 1, 2);
                char c = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
                decoded << c;
                i += 2;
            } else {
                decoded << url[i];
            }
        }
        
        return decoded.str();
    }
    
    // Parse query parameters
    std::map<std::string, std::string> ParseQueryParams(const std::string& query_string) {
        std::map<std::string, std::string> params;
        
        std::istringstream stream(query_string);
        std::string pair;
        
        while (std::getline(stream, pair, '&')) {
            size_t equal_pos = pair.find('=');
            if (equal_pos != std::string::npos) {
                std::string key = DecodeURL(pair.substr(0, equal_pos));
                std::string value = DecodeURL(pair.substr(equal_pos + 1));
                params[key] = value;
            }
        }
        
        return params;
    }
    
    // Build query string
    std::string BuildQueryString(const std::map<std::string, std::string>& params) {
        std::ostringstream query;
        bool first = true;
        
        for (const auto& param : params) {
            if (!first) query << "&";
            query << EncodeURL(param.first) << "=" << EncodeURL(param.second);
            first = false;
        }
        
        return query.str();
    }
    
    // Check if URL is valid
    bool IsValidURL(const std::string& url) {
        std::regex url_regex(R"(^https?://[^\s/$.?#].[^\s]*$)");
        return std::regex_match(url, url_regex);
    }
    
    // Extract domain from URL
    std::string ExtractDomain(const std::string& url) {
        std::regex domain_regex(R"(^https?://([^/]+))");
        std::smatch matches;
        
        if (std::regex_match(url, matches, domain_regex)) {
            return matches[1].str();
        }
        
        return "";
    }
}

} // namespace chromium_playwright::network
