#pragma once

#include "mcp_protocol.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>

namespace chromium_playwright::mcp {

// MCP Client Implementation
class MCPClientImpl : public MCPClient {
public:
    MCPClientImpl();
    explicit MCPClientImpl(const std::string& client_id);
    virtual ~MCPClientImpl();

    // MCPClient interface
    std::future<MCPResponse> send_request(const MCPRequest& request) override;
    bool send_notification(const MCPNotification& notification) override;
    void register_notification_handler(
        const std::string& method,
        std::function<void(const MCPNotification&)> handler) override;
    bool connect(const std::string& endpoint) override;
    void disconnect() override;
    bool is_connected() const override;

    // Additional methods
    void set_timeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }
    std::chrono::milliseconds timeout() const { return timeout_; }
    
    const std::string& client_id() const { return client_id_; }

private:
    // Internal message handling
    void message_loop();
    void handle_incoming_message(const std::string& message_data);
    void process_response(const MCPResponse& response);
    void process_notification(const MCPNotification& notification);
    
    // Connection management
    bool establish_connection(const std::string& endpoint);
    void close_connection();
    
    // Message queue management
    void enqueue_outgoing_message(const std::string& message);
    std::string dequeue_outgoing_message();
    void enqueue_incoming_message(const std::string& message);
    std::string dequeue_incoming_message();

private:
    std::string client_id_;
    std::string endpoint_;
    bool connected_;
    std::chrono::milliseconds timeout_;
    
    // Threading
    std::thread message_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool should_stop_;
    
    // Message queues
    std::queue<std::string> outgoing_messages_;
    std::queue<std::string> incoming_messages_;
    
    // Pending requests
    std::unordered_map<MessageId, std::promise<MCPResponse>> pending_requests_;
    
    // Notification handlers
    std::unordered_map<std::string, std::function<void(const MCPNotification&)>> notification_handlers_;
    
    // Connection details (would be replaced with actual network implementation)
    void* connection_handle_; // Placeholder for actual connection
};

// MCP Client Factory
class MCPClientFactory {
public:
    static std::unique_ptr<MCPClient> create_client();
    static std::unique_ptr<MCPClient> create_client(const std::string& client_id);
    static std::unique_ptr<MCPClient> create_client_with_endpoint(const std::string& endpoint);
};

} // namespace chromium_playwright::mcp
