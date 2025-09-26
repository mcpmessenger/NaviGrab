#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <chrono>
#include <memory>
#include <functional>
#include <future>

namespace chromium_playwright::mcp {

// Forward declarations
class MCPMessage;
class MCPResponse;
class MCPError;

// Type aliases for better readability
using MessageId = std::string;
using ModuleId = std::string;
using Timestamp = std::chrono::system_clock::time_point;
using JsonValue = std::variant<std::nullptr_t, bool, int64_t, double, std::string, 
                               std::vector<JsonValue>, std::map<std::string, JsonValue>>;

// MCP Protocol Version
constexpr int MCP_VERSION_MAJOR = 1;
constexpr int MCP_VERSION_MINOR = 0;
constexpr int MCP_VERSION_PATCH = 0;

// Message Types
enum class MessageType {
    REQUEST,
    RESPONSE,
    NOTIFICATION,
    ERROR
};

// Error Codes
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_MESSAGE = 1000,
    INVALID_PARAMETERS = 1001,
    MODULE_NOT_FOUND = 1002,
    OPERATION_FAILED = 1003,
    TIMEOUT = 1004,
    PERMISSION_DENIED = 1005,
    RESOURCE_UNAVAILABLE = 1006,
    INTERNAL_ERROR = 9999
};

// Module Types
enum class ModuleType {
    BROWSER_CONTROL,
    DOM_INTERACTION,
    SCREENSHOT_CAPTURE,
    PROACTIVE_SCRAPING,
    STORAGE_INTEGRATION,
    API_LAYER
};

// Priority levels for message processing
enum class Priority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// Base MCP Message structure
class MCPMessage {
public:
    MCPMessage(MessageType type, ModuleId target_module, std::string method);
    virtual ~MCPMessage() = default;

    // Getters
    MessageId id() const { return id_; }
    MessageType type() const { return type_; }
    ModuleId target_module() const { return target_module_; }
    std::string method() const { return method_; }
    JsonValue parameters() const { return parameters_; }
    Priority priority() const { return priority_; }
    Timestamp timestamp() const { return timestamp_; }
    std::optional<MessageId> correlation_id() const { return correlation_id_; }

    // Setters
    void set_parameters(const JsonValue& params) { parameters_ = params; }
    void set_priority(Priority priority) { priority_ = priority; }
    void set_correlation_id(const MessageId& id) { correlation_id_ = id; }

    // Serialization
    virtual std::string serialize() const = 0;
    virtual bool deserialize(const std::string& data) = 0;

    // Validation
    virtual bool is_valid() const = 0;

protected:
    MessageId id_;
    MessageType type_;
    ModuleId target_module_;
    std::string method_;
    JsonValue parameters_;
    Priority priority_;
    Timestamp timestamp_;
    std::optional<MessageId> correlation_id_;
};

// MCP Request Message
class MCPRequest : public MCPMessage {
public:
    MCPRequest(ModuleId target_module, std::string method);
    
    std::string serialize() const override;
    bool deserialize(const std::string& data) override;
    bool is_valid() const override;

    // Request-specific methods
    void set_timeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }
    std::chrono::milliseconds timeout() const { return timeout_; }

private:
    std::chrono::milliseconds timeout_{30000}; // 30 seconds default
};

// MCP Response Message
class MCPResponse : public MCPMessage {
public:
    MCPResponse(const MCPRequest& request, const JsonValue& result);
    MCPResponse(const MCPRequest& request, const MCPError& error);
    
    std::string serialize() const override;
    bool deserialize(const std::string& data) override;
    bool is_valid() const override;

    // Response-specific methods
    bool is_success() const { return error_code_ == ErrorCode::SUCCESS; }
    ErrorCode error_code() const { return error_code_; }
    std::string error_message() const { return error_message_; }
    JsonValue result() const { return result_; }

private:
    ErrorCode error_code_;
    std::string error_message_;
    JsonValue result_;
};

// MCP Error Message
class MCPError {
public:
    MCPError(ErrorCode code, const std::string& message);
    MCPError(ErrorCode code, const std::string& message, const JsonValue& details);

    ErrorCode code() const { return code_; }
    std::string message() const { return message_; }
    JsonValue details() const { return details_; }
    std::string serialize() const;

private:
    ErrorCode code_;
    std::string message_;
    JsonValue details_;
};

// MCP Notification Message
class MCPNotification : public MCPMessage {
public:
    MCPNotification(ModuleId target_module, std::string method);
    
    std::string serialize() const override;
    bool deserialize(const std::string& data) override;
    bool is_valid() const override;
};

// MCP Client Interface
class MCPClient {
public:
    virtual ~MCPClient() = default;

    // Send request and wait for response
    virtual std::future<MCPResponse> send_request(const MCPRequest& request) = 0;
    
    // Send notification (fire and forget)
    virtual bool send_notification(const MCPNotification& notification) = 0;
    
    // Register callback for incoming notifications
    virtual void register_notification_handler(
        const std::string& method,
        std::function<void(const MCPNotification&)> handler) = 0;
    
    // Connect to MCP server
    virtual bool connect(const std::string& endpoint) = 0;
    
    // Disconnect from MCP server
    virtual void disconnect() = 0;
    
    // Check if connected
    virtual bool is_connected() const = 0;
};

// MCP Server Interface
class MCPServer {
public:
    virtual ~MCPServer() = default;

    // Register request handler
    virtual void register_request_handler(
        const std::string& method,
        std::function<MCPResponse(const MCPRequest&)> handler) = 0;
    
    // Register notification handler
    virtual void register_notification_handler(
        const std::string& method,
        std::function<void(const MCPNotification&)> handler) = 0;
    
    // Start server
    virtual bool start(const std::string& endpoint) = 0;
    
    // Stop server
    virtual void stop() = 0;
    
    // Check if running
    virtual bool is_running() const = 0;
    
    // Send notification to client
    virtual bool send_notification(const std::string& client_id, const MCPNotification& notification) = 0;
};

// MCP Message Router
class MCPRouter {
public:
    MCPRouter();
    virtual ~MCPRouter() = default;

    // Register module
    virtual void register_module(ModuleId module_id, ModuleType type, std::shared_ptr<MCPClient> client) = 0;
    
    // Unregister module
    virtual void unregister_module(ModuleId module_id) = 0;
    
    // Route message to appropriate module
    virtual bool route_message(const MCPMessage& message) = 0;
    
    // Broadcast notification to all modules
    virtual void broadcast_notification(const MCPNotification& notification) = 0;
    
    // Get module by ID
    virtual std::shared_ptr<MCPClient> get_module(ModuleId module_id) const = 0;
    
    // List all registered modules
    virtual std::vector<ModuleId> list_modules() const = 0;
};

// Utility functions
namespace utils {
    // Generate unique message ID
    MessageId generate_message_id();
    
    // Get current timestamp
    Timestamp get_current_timestamp();
    
    // Convert error code to string
    std::string error_code_to_string(ErrorCode code);
    
    // Convert module type to string
    std::string module_type_to_string(ModuleType type);
    
    // JSON serialization helpers
    std::string json_to_string(const JsonValue& value);
    JsonValue string_to_json(const std::string& str);
    
    // Message validation
    bool validate_message(const MCPMessage& message);
}

} // namespace chromium_playwright::mcp
