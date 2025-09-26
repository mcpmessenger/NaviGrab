#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "chromium_playwright/mcp/mcp_protocol.h"
#include <chrono>
#include <thread>

using namespace chromium_playwright::mcp;
using namespace testing;

class MCPProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test fixtures
    }
    
    void TearDown() override {
        // Clean up test fixtures
    }
};

TEST_F(MCPProtocolTest, MessageIdGeneration) {
    auto id1 = utils::generate_message_id();
    auto id2 = utils::generate_message_id();
    
    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());
    EXPECT_NE(id1, id2);
}

TEST_F(MCPProtocolTest, TimestampGeneration) {
    auto timestamp1 = utils::get_current_timestamp();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto timestamp2 = utils::get_current_timestamp();
    
    EXPECT_LT(timestamp1, timestamp2);
}

TEST_F(MCPProtocolTest, ErrorCodeToString) {
    EXPECT_EQ(utils::error_code_to_string(ErrorCode::SUCCESS), "SUCCESS");
    EXPECT_EQ(utils::error_code_to_string(ErrorCode::INVALID_MESSAGE), "INVALID_MESSAGE");
    EXPECT_EQ(utils::error_code_to_string(ErrorCode::TIMEOUT), "TIMEOUT");
}

TEST_F(MCPProtocolTest, ModuleTypeToString) {
    EXPECT_EQ(utils::module_type_to_string(ModuleType::BROWSER_CONTROL), "BROWSER_CONTROL");
    EXPECT_EQ(utils::module_type_to_string(ModuleType::DOM_INTERACTION), "DOM_INTERACTION");
    EXPECT_EQ(utils::module_type_to_string(ModuleType::SCREENSHOT_CAPTURE), "SCREENSHOT_CAPTURE");
}

TEST_F(MCPProtocolTest, MCPRequestCreation) {
    MCPRequest request("browser_control", "navigate");
    
    EXPECT_EQ(request.target_module(), "browser_control");
    EXPECT_EQ(request.method(), "navigate");
    EXPECT_EQ(request.type(), MessageType::REQUEST);
    EXPECT_FALSE(request.id().empty());
    EXPECT_TRUE(request.is_valid());
}

TEST_F(MCPProtocolTest, MCPRequestWithParameters) {
    MCPRequest request("dom_interaction", "click");
    
    JsonValue params = std::map<std::string, JsonValue>{
        {"selector", std::string("button")},
        {"timeout", 5000}
    };
    request.set_parameters(params);
    
    EXPECT_EQ(request.parameters(), params);
    EXPECT_TRUE(request.is_valid());
}

TEST_F(MCPProtocolTest, MCPResponseCreation) {
    MCPRequest request("browser_control", "navigate");
    JsonValue result = std::string("success");
    MCPResponse response(request, result);
    
    EXPECT_EQ(response.target_module(), "browser_control");
    EXPECT_EQ(response.method(), "navigate");
    EXPECT_EQ(response.type(), MessageType::RESPONSE);
    EXPECT_TRUE(response.is_success());
    EXPECT_EQ(response.error_code(), ErrorCode::SUCCESS);
    EXPECT_EQ(response.result(), result);
}

TEST_F(MCPProtocolTest, MCPResponseWithError) {
    MCPRequest request("browser_control", "navigate");
    MCPError error(ErrorCode::INVALID_PARAMETERS, "Invalid URL provided");
    MCPResponse response(request, error);
    
    EXPECT_FALSE(response.is_success());
    EXPECT_EQ(response.error_code(), ErrorCode::INVALID_PARAMETERS);
    EXPECT_EQ(response.error_message(), "Invalid URL provided");
}

TEST_F(MCPProtocolTest, MCPNotificationCreation) {
    MCPNotification notification("screenshot_capture", "screenshot_completed");
    
    EXPECT_EQ(notification.target_module(), "screenshot_capture");
    EXPECT_EQ(notification.method(), "screenshot_completed");
    EXPECT_EQ(notification.type(), MessageType::NOTIFICATION);
    EXPECT_TRUE(notification.is_valid());
}

TEST_F(MCPProtocolTest, MCPErrorCreation) {
    MCPError error(ErrorCode::OPERATION_FAILED, "Screenshot capture failed");
    
    EXPECT_EQ(error.code(), ErrorCode::OPERATION_FAILED);
    EXPECT_EQ(error.message(), "Screenshot capture failed");
    
    auto serialized = error.serialize();
    EXPECT_FALSE(serialized.empty());
}

TEST_F(MCPProtocolTest, MessageSerialization) {
    MCPRequest request("browser_control", "navigate");
    JsonValue params = std::map<std::string, JsonValue>{
        {"url", std::string("https://example.com")},
        {"timeout", 30000}
    };
    request.set_parameters(params);
    
    auto serialized = request.serialize();
    EXPECT_FALSE(serialized.empty());
    
    MCPRequest deserialized_request("", "");
    EXPECT_TRUE(deserialized_request.deserialize(serialized));
    EXPECT_EQ(deserialized_request.target_module(), "browser_control");
    EXPECT_EQ(deserialized_request.method(), "navigate");
}

TEST_F(MCPProtocolTest, JSONSerialization) {
    JsonValue value = std::map<std::string, JsonValue>{
        {"string", std::string("test")},
        {"number", 42},
        {"boolean", true},
        {"array", std::vector<JsonValue>{1, 2, 3}},
        {"null", nullptr}
    };
    
    auto json_string = utils::json_to_string(value);
    EXPECT_FALSE(json_string.empty());
    
    auto parsed_value = utils::string_to_json(json_string);
    EXPECT_EQ(parsed_value, value);
}

TEST_F(MCPProtocolTest, MessageValidation) {
    MCPRequest valid_request("browser_control", "navigate");
    EXPECT_TRUE(utils::validate_message(valid_request));
    
    MCPRequest invalid_request("", ""); // Empty module and method
    EXPECT_FALSE(utils::validate_message(invalid_request));
}

TEST_F(MCPProtocolTest, PriorityHandling) {
    MCPRequest request("browser_control", "navigate");
    
    request.set_priority(Priority::HIGH);
    EXPECT_EQ(request.priority(), Priority::HIGH);
    
    request.set_priority(Priority::CRITICAL);
    EXPECT_EQ(request.priority(), Priority::CRITICAL);
}

TEST_F(MCPProtocolTest, CorrelationIdHandling) {
    MCPRequest request("browser_control", "navigate");
    std::string correlation_id = "corr_123";
    
    request.set_correlation_id(correlation_id);
    EXPECT_EQ(request.correlation_id(), correlation_id);
}

TEST_F(MCPProtocolTest, TimeoutHandling) {
    MCPRequest request("browser_control", "navigate");
    
    auto timeout = std::chrono::milliseconds(5000);
    request.set_timeout(timeout);
    EXPECT_EQ(request.timeout(), timeout);
}

// Test MCP Client functionality
class MCPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client_ = MCPClientFactory::create_client("test_client");
    }
    
    void TearDown() override {
        if (client_) {
            client_->disconnect();
        }
    }
    
    std::unique_ptr<MCPClient> client_;
};

TEST_F(MCPClientTest, ClientCreation) {
    EXPECT_NE(client_, nullptr);
    EXPECT_FALSE(client_->is_connected());
}

TEST_F(MCPClientTest, ClientConnection) {
    // Note: In a real test, you'd need a mock server
    // For now, we'll test the interface
    EXPECT_FALSE(client_->is_connected());
    
    // Test connection attempt (will fail without real server)
    EXPECT_FALSE(client_->connect("localhost:9999"));
}

// Test MCP Server functionality
class MCPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_ = MCPServerFactory::create_server("test_server");
    }
    
    void TearDown() override {
        if (server_) {
            server_->stop();
        }
    }
    
    std::unique_ptr<MCPServer> server_;
};

TEST_F(MCPServerTest, ServerCreation) {
    EXPECT_NE(server_, nullptr);
    EXPECT_FALSE(server_->is_running());
}

TEST_F(MCPServerTest, ServerStartStop) {
    // Test server start (will fail without proper network setup)
    EXPECT_FALSE(server_->start("localhost:9999"));
    EXPECT_FALSE(server_->is_running());
}

// Test MCP Router functionality
class MCPRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_ = std::make_unique<MCPRouter>();
    }
    
    void TearDown() override {
        // Clean up
    }
    
    std::unique_ptr<MCPRouter> router_;
};

TEST_F(MCPRouterTest, RouterCreation) {
    EXPECT_NE(router_, nullptr);
}

TEST_F(MCPRouterTest, ModuleRegistration) {
    auto client = MCPClientFactory::create_client("test_module");
    
    // Note: In a real implementation, you'd test module registration
    // For now, we'll test the interface
    EXPECT_NE(client, nullptr);
}

// Performance tests
TEST_F(MCPProtocolTest, MessageSerializationPerformance) {
    const int iterations = 1000;
    MCPRequest request("browser_control", "navigate");
    JsonValue params = std::map<std::string, JsonValue>{
        {"url", std::string("https://example.com")},
        {"timeout", 30000},
        {"data", std::vector<JsonValue>{1, 2, 3, 4, 5}}
    };
    request.set_parameters(params);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        auto serialized = request.serialize();
        MCPRequest deserialized("", "");
        deserialized.deserialize(serialized);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Serialized " << iterations << " messages in " 
              << duration.count() << " microseconds" << std::endl;
    
    // Should be reasonably fast
    EXPECT_LT(duration.count(), 1000000); // Less than 1 second
}

// Integration tests
TEST_F(MCPProtocolTest, EndToEndMessageFlow) {
    // Create request
    MCPRequest request("browser_control", "navigate");
    JsonValue params = std::map<std::string, JsonValue>{
        {"url", std::string("https://example.com")}
    };
    request.set_parameters(params);
    
    // Serialize request
    auto serialized_request = request.serialize();
    EXPECT_FALSE(serialized_request.empty());
    
    // Deserialize request
    MCPRequest deserialized_request("", "");
    EXPECT_TRUE(deserialized_request.deserialize(serialized_request));
    EXPECT_EQ(deserialized_request.target_module(), "browser_control");
    EXPECT_EQ(deserialized_request.method(), "navigate");
    
    // Create response
    JsonValue result = std::string("navigation_successful");
    MCPResponse response(deserialized_request, result);
    
    // Serialize response
    auto serialized_response = response.serialize();
    EXPECT_FALSE(serialized_response.empty());
    
    // Deserialize response
    MCPResponse deserialized_response(deserialized_request, result);
    EXPECT_TRUE(deserialized_response.deserialize(serialized_response));
    EXPECT_TRUE(deserialized_response.is_success());
    EXPECT_EQ(deserialized_response.result(), result);
}
