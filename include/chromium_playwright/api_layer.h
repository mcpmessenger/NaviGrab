#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <optional>
#include <future>
#include "browser_control.h"
#include "dom_interaction.h"
#include "screenshot_capture.h"
#include "proactive_scraping.h"
#include "storage_integration.h"
#include "mcp/mcp_protocol.h"

namespace chromium_playwright {

// Forward declarations
class APILayer;
class MojoInterface;

// API configuration
struct APIConfig {
    std::string endpoint = "localhost:8080";
    int max_connections = 100;
    std::chrono::milliseconds request_timeout{30000};
    std::chrono::milliseconds connection_timeout{10000};
    bool enable_compression = true;
    bool enable_encryption = false;
    std::string api_key = "";
    std::map<std::string, std::string> custom_headers;
    std::string log_level = "info";
    std::string log_file = "";
};

// API request/response types
struct APIRequest {
    std::string id;
    std::string method;
    std::map<std::string, std::string> parameters;
    std::map<std::string, std::string> headers;
    std::chrono::system_clock::time_point timestamp;
    int priority = 0;
    std::string correlation_id = "";
};

struct APIResponse {
    std::string id;
    bool success = false;
    std::string error_message = "";
    std::map<std::string, std::string> data;
    std::map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::milliseconds processing_time{0};
    std::string correlation_id = "";
};

// API event types
enum class APIEvent {
    CONNECTION_ESTABLISHED,
    CONNECTION_LOST,
    REQUEST_RECEIVED,
    RESPONSE_SENT,
    ERROR_OCCURRED,
    SESSION_STARTED,
    SESSION_ENDED
};

// API event handler
using APIEventHandler = std::function<void(APIEvent event, const std::map<std::string, std::string>& data)>;

// API Layer Interface
class APILayer {
public:
    virtual ~APILayer() = default;

    // Initialization
    virtual bool Initialize(const APIConfig& config) = 0;
    virtual bool IsInitialized() const = 0;
    virtual void Shutdown() = 0;

    // Request handling
    virtual std::future<APIResponse> SendRequest(const APIRequest& request) = 0;
    virtual void RegisterRequestHandler(const std::string& method, 
                                      std::function<APIResponse(const APIRequest&)> handler) = 0;
    virtual void UnregisterRequestHandler(const std::string& method) = 0;

    // Event handling
    virtual void RegisterEventHandler(APIEvent event, APIEventHandler handler) = 0;
    virtual void UnregisterEventHandler(APIEvent event) = 0;
    virtual void EmitEvent(APIEvent event, const std::map<std::string, std::string>& data) = 0;

    // Connection management
    virtual bool StartServer() = 0;
    virtual bool StopServer() = 0;
    virtual bool IsServerRunning() const = 0;
    virtual std::vector<std::string> GetConnectedClients() const = 0;
    virtual bool DisconnectClient(const std::string& client_id) = 0;

    // Configuration
    virtual APIConfig GetConfig() const = 0;
    virtual bool UpdateConfig(const APIConfig& config) = 0;
    virtual void SetLogLevel(const std::string& level) = 0;
    virtual std::string GetLogLevel() const = 0;

    // Statistics
    virtual std::map<std::string, int> GetRequestStatistics() const = 0;
    virtual std::map<std::string, double> GetPerformanceMetrics() const = 0;
    virtual std::vector<std::string> GetErrorLog() const = 0;
    virtual void ClearStatistics() = 0;
};

// Mojo Interface for Chromium Integration
class MojoInterface {
public:
    virtual ~MojoInterface() = default;

    // Mojo interface registration
    virtual bool RegisterInterface(const std::string& interface_name, 
                                 std::shared_ptr<void> interface_impl) = 0;
    virtual bool UnregisterInterface(const std::string& interface_name) = 0;
    virtual std::vector<std::string> GetRegisteredInterfaces() const = 0;

    // Mojo message handling
    virtual bool SendMojoMessage(const std::string& interface_name, 
                               const std::string& method_name,
                               const std::map<std::string, std::string>& parameters) = 0;
    virtual void RegisterMojoMessageHandler(const std::string& interface_name,
                                          const std::string& method_name,
                                          std::function<void(const std::map<std::string, std::string>&)> handler) = 0;

    // Process communication
    virtual bool SendToRendererProcess(const std::string& message) = 0;
    virtual bool SendToBrowserProcess(const std::string& message) = 0;
    virtual void RegisterRendererMessageHandler(std::function<void(const std::string&)> handler) = 0;
    virtual void RegisterBrowserMessageHandler(std::function<void(const std::string&)> handler) = 0;

    // Connection status
    virtual bool IsRendererConnected() const = 0;
    virtual bool IsBrowserConnected() const = 0;
    virtual void WaitForRendererConnection() = 0;
    virtual void WaitForBrowserConnection() = 0;
};

// REST API Interface
class RESTAPI {
public:
    virtual ~RESTAPI() = default;

    // HTTP methods
    virtual APIResponse Get(const std::string& endpoint, 
                          const std::map<std::string, std::string>& parameters = {},
                          const std::map<std::string, std::string>& headers = {}) = 0;
    virtual APIResponse Post(const std::string& endpoint, 
                           const std::map<std::string, std::string>& data = {},
                           const std::map<std::string, std::string>& headers = {}) = 0;
    virtual APIResponse Put(const std::string& endpoint, 
                          const std::map<std::string, std::string>& data = {},
                          const std::map<std::string, std::string>& headers = {}) = 0;
    virtual APIResponse Delete(const std::string& endpoint, 
                             const std::map<std::string, std::string>& parameters = {},
                             const std::map<std::string, std::string>& headers = {}) = 0;

    // Async HTTP methods
    virtual std::future<APIResponse> GetAsync(const std::string& endpoint, 
                                            const std::map<std::string, std::string>& parameters = {},
                                            const std::map<std::string, std::string>& headers = {}) = 0;
    virtual std::future<APIResponse> PostAsync(const std::string& endpoint, 
                                             const std::map<std::string, std::string>& data = {},
                                             const std::map<std::string, std::string>& headers = {}) = 0;

    // Route registration
    virtual void RegisterRoute(const std::string& method, const std::string& path,
                             std::function<APIResponse(const APIRequest&)> handler) = 0;
    virtual void UnregisterRoute(const std::string& method, const std::string& path) = 0;
    virtual std::vector<std::pair<std::string, std::string>> GetRegisteredRoutes() const = 0;

    // Middleware
    virtual void AddMiddleware(std::function<bool(const APIRequest&, APIResponse&)> middleware) = 0;
    virtual void RemoveMiddleware(int index) = 0;
    virtual std::vector<std::function<bool(const APIRequest&, APIResponse&)>> GetMiddlewares() const = 0;
};

// WebSocket API Interface
class WebSocketAPI {
public:
    virtual ~WebSocketAPI() = default;

    // Connection management
    virtual bool Connect(const std::string& url) = 0;
    virtual bool Disconnect() = 0;
    virtual bool IsConnected() const = 0;

    // Message sending
    virtual bool SendMessage(const std::string& message) = 0;
    virtual bool SendBinaryMessage(const std::vector<uint8_t>& data) = 0;
    virtual bool SendJsonMessage(const std::map<std::string, std::string>& data) = 0;

    // Message handling
    virtual void RegisterMessageHandler(std::function<void(const std::string&)> handler) = 0;
    virtual void RegisterBinaryMessageHandler(std::function<void(const std::vector<uint8_t>&)> handler) = 0;
    virtual void RegisterJsonMessageHandler(std::function<void(const std::map<std::string, std::string>&)> handler) = 0;

    // Event handling
    virtual void OnOpen(std::function<void()> handler) = 0;
    virtual void OnClose(std::function<void(int code, const std::string& reason)> handler) = 0;
    virtual void OnError(std::function<void(const std::string& error)> handler) = 0;
};

// GraphQL API Interface
class GraphQLAPI {
public:
    virtual ~GraphQLAPI() = default;

    // Query execution
    virtual APIResponse ExecuteQuery(const std::string& query, 
                                   const std::map<std::string, std::string>& variables = {}) = 0;
    virtual std::future<APIResponse> ExecuteQueryAsync(const std::string& query, 
                                                     const std::map<std::string, std::string>& variables = {}) = 0;

    // Schema management
    virtual bool LoadSchema(const std::string& schema_definition) = 0;
    virtual std::string GetSchema() const = 0;
    virtual bool ValidateQuery(const std::string& query) const = 0;

    // Resolver registration
    virtual void RegisterResolver(const std::string& field_name, 
                                std::function<std::map<std::string, std::string>(const std::map<std::string, std::string>&)> resolver) = 0;
    virtual void UnregisterResolver(const std::string& field_name) = 0;
    virtual std::vector<std::string> GetRegisteredResolvers() const = 0;
};

// API Factory
class APIFactory {
public:
    static std::unique_ptr<APILayer> CreateAPILayer();
    static std::unique_ptr<APILayer> CreateAPILayer(const APIConfig& config);
    static std::unique_ptr<MojoInterface> CreateMojoInterface();
    static std::unique_ptr<RESTAPI> CreateRESTAPI();
    static std::unique_ptr<WebSocketAPI> CreateWebSocketAPI();
    static std::unique_ptr<GraphQLAPI> CreateGraphQLAPI();
};

// API Utilities
namespace api_utils {
    // Request/Response helpers
    APIRequest CreateRequest(const std::string& method, const std::map<std::string, std::string>& parameters = {});
    APIResponse CreateSuccessResponse(const std::string& request_id, const std::map<std::string, std::string>& data = {});
    APIResponse CreateErrorResponse(const std::string& request_id, const std::string& error_message);
    
    // Serialization
    std::string SerializeRequest(const APIRequest& request);
    APIRequest DeserializeRequest(const std::string& data);
    std::string SerializeResponse(const APIResponse& response);
    APIResponse DeserializeResponse(const std::string& data);
    
    // Validation
    bool ValidateRequest(const APIRequest& request);
    bool ValidateResponse(const APIResponse& response);
    std::vector<std::string> GetRequestValidationErrors(const APIRequest& request);
    std::vector<std::string> GetResponseValidationErrors(const APIResponse& response);
    
    // Error handling
    std::string GetErrorMessage(int error_code);
    int GetErrorCode(const std::string& error_message);
    bool IsRetryableError(int error_code);
    
    // Logging
    void LogRequest(const APIRequest& request);
    void LogResponse(const APIResponse& response);
    void LogError(const std::string& message, const std::map<std::string, std::string>& context = {});
}

} // namespace chromium_playwright
