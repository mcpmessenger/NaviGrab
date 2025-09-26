#pragma once

#include "mcp_protocol.h"
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <set>

namespace chromium_playwright::mcp {

// MCP Server Implementation
class MCPServerImpl : public MCPServer {
public:
    MCPServerImpl();
    explicit MCPServerImpl(const std::string& server_id);
    virtual ~MCPServerImpl();

    // MCPServer interface
    void register_request_handler(
        const std::string& method,
        std::function<MCPResponse(const MCPRequest&)> handler) override;
    void register_notification_handler(
        const std::string& method,
        std::function<void(const MCPNotification&)> handler) override;
    bool start(const std::string& endpoint) override;
    void stop() override;
    bool is_running() const override;
    bool send_notification(const std::string& client_id, const MCPNotification& notification) override;

    // Additional methods
    const std::string& server_id() const { return server_id_; }
    std::vector<std::string> connected_clients() const;
    void disconnect_client(const std::string& client_id);
    void set_max_clients(size_t max_clients) { max_clients_ = max_clients; }
    size_t max_clients() const { return max_clients_; }

private:
    // Internal message handling
    void message_loop();
    void handle_incoming_message(const std::string& client_id, const std::string& message_data);
    void process_request(const std::string& client_id, const MCPRequest& request);
    void process_notification(const std::string& client_id, const MCPNotification& notification);
    
    // Client management
    void add_client(const std::string& client_id, void* connection_handle);
    void remove_client(const std::string& client_id);
    bool is_client_connected(const std::string& client_id) const;
    void* get_client_connection(const std::string& client_id) const;
    
    // Message sending
    bool send_message_to_client(const std::string& client_id, const std::string& message);
    void broadcast_message(const std::string& message, const std::set<std::string>& exclude_clients = {});

private:
    std::string server_id_;
    std::string endpoint_;
    bool running_;
    size_t max_clients_;
    
    // Threading
    std::thread message_thread_;
    std::mutex mutex_;
    bool should_stop_;
    
    // Client management
    std::unordered_map<std::string, void*> clients_; // client_id -> connection_handle
    std::set<std::string> connected_client_ids_;
    
    // Request handlers
    std::unordered_map<std::string, std::function<MCPResponse(const MCPRequest&)>> request_handlers_;
    
    // Notification handlers
    std::unordered_map<std::string, std::function<void(const MCPNotification&)>> notification_handlers_;
    
    // Server details (would be replaced with actual network implementation)
    void* server_handle_; // Placeholder for actual server
};

// MCP Server Factory
class MCPServerFactory {
public:
    static std::unique_ptr<MCPServer> create_server();
    static std::unique_ptr<MCPServer> create_server(const std::string& server_id);
    static std::unique_ptr<MCPServer> create_server_with_endpoint(const std::string& endpoint);
};

} // namespace chromium_playwright::mcp
