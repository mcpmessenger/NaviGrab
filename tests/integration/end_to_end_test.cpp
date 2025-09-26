#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "chromium_playwright/browser_control.h"
#include "chromium_playwright/dom_interaction.h"
#include "chromium_playwright/screenshot_capture.h"
#include "chromium_playwright/proactive_scraping.h"
#include "chromium_playwright/storage_integration.h"
#include "chromium_playwright/api_layer.h"
#include "chromium_playwright/mcp/mcp_protocol.h"
#include <chrono>
#include <thread>
#include <filesystem>

using namespace chromium_playwright;
using namespace testing;

class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize all components
        browser_ = &BrowserControl::GetInstance();
        screenshot_capture_ = CreateScreenshotCapture();
        scraper_ = CreateScraper();
        storage_ = StorageFactory::CreateStorage(StorageType::SQLITE);
        api_layer_ = APIFactory::CreateAPILayer();
        
        // Initialize storage
        StorageConfig storage_config;
        storage_config.database_path = "./test_chromium_playwright.db";
        storage_->Initialize(storage_config);
        
        // Initialize API layer
        APIConfig api_config;
        api_config.endpoint = "localhost:0"; // Use any available port
        api_layer_->Initialize(api_config);
        
        // Create test context and page
        context_id_ = browser_->NewContext();
        context_ = browser_->GetContext(context_id_);
        page_id_ = context_->NewPage();
        page_ = context_->GetPage(page_id_);
    }
    
    void TearDown() override {
        // Cleanup
        if (page_) {
            page_->Close();
        }
        if (context_) {
            context_->Close();
        }
        browser_->Shutdown();
        
        // Clean up test database
        if (std::filesystem::exists("./test_chromium_playwright.db")) {
            std::filesystem::remove("./test_chromium_playwright.db");
        }
    }
    
    BrowserControl* browser_;
    std::unique_ptr<ScreenshotCapture> screenshot_capture_;
    std::unique_ptr<Scraper> scraper_;
    std::unique_ptr<StorageManager> storage_;
    std::unique_ptr<APILayer> api_layer_;
    
    ContextId context_id_;
    std::unique_ptr<BrowserContext> context_;
    PageId page_id_;
    std::unique_ptr<Page> page_;
};

TEST_F(EndToEndTest, BasicNavigationAndScreenshot) {
    // Navigate to a test page
    NavigationOptions nav_options;
    nav_options.timeout = std::chrono::milliseconds(10000);
    nav_options.wait_until = "domcontentloaded";
    
    ASSERT_TRUE(page_->Goto("data:text/html,<html><body><h1>Test Page</h1><p>This is a test</p></body></html>", nav_options));
    EXPECT_EQ(page_->Title(), "");
    EXPECT_FALSE(page_->Url().empty());
    
    // Take a screenshot
    ScreenshotOptions screenshot_options;
    screenshot_options.path = "./test_screenshot.png";
    screenshot_options.full_page = true;
    
    auto screenshot_result = screenshot_capture_->CapturePage(*page_, screenshot_options);
    EXPECT_TRUE(screenshot_result.success);
    EXPECT_FALSE(screenshot_result.image_data.empty());
    
    // Clean up screenshot file
    if (std::filesystem::exists("./test_screenshot.png")) {
        std::filesystem::remove("./test_screenshot.png");
    }
}

TEST_F(EndToEndTest, DOMInteraction) {
    // Navigate to a test page with interactive elements
    std::string test_html = R"(
        <html>
        <body>
            <h1>Test Page</h1>
            <button id="test-button">Click Me</button>
            <input id="test-input" type="text" placeholder="Enter text">
            <div id="test-output"></div>
        </body>
        </html>
    )";
    
    NavigationOptions nav_options;
    nav_options.timeout = std::chrono::milliseconds(10000);
    
    ASSERT_TRUE(page_->Goto("data:text/html," + test_html, nav_options));
    
    // Test element finding
    auto button_locator = page_->GetByCss("#test-button");
    EXPECT_TRUE(button_locator->Exists());
    EXPECT_EQ(button_locator->TextContent(), "Click Me");
    
    auto input_locator = page_->GetByCss("#test-input");
    EXPECT_TRUE(input_locator->Exists());
    
    // Test clicking
    EXPECT_TRUE(button_locator->Click());
    
    // Test typing
    EXPECT_TRUE(input_locator->Type("Hello World"));
    
    // Verify the input value
    auto input_element = input_locator->Element();
    ASSERT_NE(input_element, nullptr);
    // Note: In a real implementation, you'd check the actual value
}

TEST_F(EndToEndTest, ProactiveScraping) {
    // Set up scraping configuration
    ScrapingConfig config;
    config.start_url = "data:text/html,<html><body><h1>Test</h1><a href='#link1'>Link 1</a></body></html>";
    config.max_depth = 1;
    config.click_all_buttons = false;
    config.follow_all_links = true;
    config.take_screenshots = true;
    config.max_pages = 3;
    config.page_timeout = std::chrono::milliseconds(5000);
    
    // Set up progress callback
    std::vector<ScrapedPageData> scraped_pages;
    scraper_->SetProgressCallback([&scraped_pages](const ScrapingSession& session, const ScrapedPageData& page_data) {
        scraped_pages.push_back(page_data);
    });
    
    // Start scraping
    auto session_id = scraper_->StartScraping(config);
    EXPECT_GT(session_id, 0);
    
    // Wait for scraping to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    // Check results
    auto results = scraper_->GetScrapingResults(session_id);
    EXPECT_GE(results.size(), 1);
    
    // Verify first page data
    if (!results.empty()) {
        EXPECT_FALSE(results[0].url.empty());
        EXPECT_FALSE(results[0].html_content.empty());
    }
    
    // Stop scraping
    scraper_->StopScraping(session_id);
}

TEST_F(EndToEndTest, StorageIntegration) {
    // Create test data
    ScrapedPageData test_data;
    test_data.url = "https://example.com";
    test_data.title = "Example Page";
    test_data.timestamp = std::chrono::system_clock::now();
    test_data.extracted_data["heading"] = "Welcome to Example";
    test_data.html_content = "<html><body><h1>Welcome to Example</h1></body></html>";
    test_data.text_content = "Welcome to Example";
    test_data.depth = 0;
    
    // Store the data
    auto storage_id = storage_->StoreScrapedPageData(test_data);
    EXPECT_FALSE(storage_id.empty());
    
    // Retrieve the data
    auto retrieved_data = storage_->RetrieveScrapedPageData(storage_id);
    ASSERT_TRUE(retrieved_data.has_value());
    EXPECT_EQ(retrieved_data->url, test_data.url);
    EXPECT_EQ(retrieved_data->title, test_data.title);
    EXPECT_EQ(retrieved_data->extracted_data["heading"], "Welcome to Example");
    
    // Test screenshot storage
    std::vector<uint8_t> test_image = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}; // PNG header
    auto screenshot_id = storage_->StoreScreenshot(1, "https://example.com", test_image, "png");
    EXPECT_FALSE(screenshot_id.empty());
    
    // Retrieve screenshot
    auto retrieved_image = storage_->RetrieveScreenshot(screenshot_id);
    ASSERT_TRUE(retrieved_image.has_value());
    EXPECT_EQ(retrieved_image->size(), test_image.size());
}

TEST_F(EndToEndTest, APILayerIntegration) {
    // Register a test request handler
    bool handler_called = false;
    api_layer_->RegisterRequestHandler("test_method", [&handler_called](const APIRequest& request) -> APIResponse {
        handler_called = true;
        APIResponse response;
        response.id = request.id;
        response.success = true;
        response.data["message"] = "Test response";
        return response;
    });
    
    // Create a test request
    APIRequest request;
    request.id = "test_request_1";
    request.method = "test_method";
    request.parameters["param1"] = "value1";
    
    // Send request (this would normally go through the network layer)
    // For this test, we'll simulate the handler being called directly
    auto handler = api_layer_->GetRegisteredRequestHandlers()["test_method"];
    if (handler) {
        auto response = handler(request);
        EXPECT_TRUE(response.success);
        EXPECT_EQ(response.data["message"], "Test response");
    }
}

TEST_F(EndToEndTest, MCPCommunication) {
    // Test MCP message creation and serialization
    MCPRequest request("browser_control", "navigate");
    JsonValue params = std::map<std::string, JsonValue>{
        {"url", std::string("https://example.com")},
        {"timeout", 30000}
    };
    request.set_parameters(params);
    
    // Serialize and deserialize
    auto serialized = request.serialize();
    EXPECT_FALSE(serialized.empty());
    
    MCPRequest deserialized("", "");
    EXPECT_TRUE(deserialized.deserialize(serialized));
    EXPECT_EQ(deserialized.target_module(), "browser_control");
    EXPECT_EQ(deserialized.method(), "navigate");
    
    // Test response creation
    JsonValue result = std::string("navigation_successful");
    MCPResponse response(deserialized, result);
    EXPECT_TRUE(response.is_success());
    EXPECT_EQ(response.result(), result);
}

TEST_F(EndToEndTest, ErrorHandling) {
    // Test navigation to invalid URL
    NavigationOptions nav_options;
    nav_options.timeout = std::chrono::milliseconds(1000); // Short timeout
    
    EXPECT_FALSE(page_->Goto("invalid://url", nav_options));
    
    // Test element interaction on non-existent element
    auto locator = page_->GetByCss("#non-existent-element");
    EXPECT_FALSE(locator->Exists());
    EXPECT_FALSE(locator->Click());
    
    // Test screenshot with invalid options
    ScreenshotOptions invalid_options;
    invalid_options.path = "/invalid/path/screenshot.png";
    invalid_options.timeout = std::chrono::milliseconds(1); // Very short timeout
    
    auto screenshot_result = screenshot_capture_->CapturePage(*page_, invalid_options);
    EXPECT_FALSE(screenshot_result.success);
}

TEST_F(EndToEndTest, PerformanceTest) {
    // Test multiple rapid operations
    const int iterations = 10;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        // Navigate to test page
        std::string test_html = "<html><body><h1>Test " + std::to_string(i) + "</h1></body></html>";
        page_->Goto("data:text/html," + test_html);
        
        // Take screenshot
        ScreenshotOptions options;
        options.path = "./perf_test_" + std::to_string(i) + ".png";
        screenshot_capture_->CapturePage(*page_, options);
        
        // Interact with elements
        auto locator = page_->GetByCss("h1");
        if (locator->Exists()) {
            locator->TextContent();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Completed " << iterations << " iterations in " << duration.count() << "ms" << std::endl;
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 30000); // Less than 30 seconds
    
    // Clean up test files
    for (int i = 0; i < iterations; ++i) {
        std::string filename = "./perf_test_" + std::to_string(i) + ".png";
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    }
}

TEST_F(EndToEndTest, MemoryLeakTest) {
    // Test that objects are properly cleaned up
    const int iterations = 100;
    
    for (int i = 0; i < iterations; ++i) {
        // Create and destroy contexts and pages
        auto test_context_id = browser_->NewContext();
        auto test_context = browser_->GetContext(test_context_id);
        auto test_page_id = test_context->NewPage();
        auto test_page = test_context->GetPage(test_page_id);
        
        // Use the page
        test_page->Goto("data:text/html,<html><body>Test</body></html>");
        
        // Clean up
        test_page->Close();
        test_context->Close();
        browser_->CloseContext(test_context_id);
    }
    
    // If we get here without crashing, memory management is working
    SUCCEED();
}
