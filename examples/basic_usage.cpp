#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include "chromium_playwright/browser_control.h"
#include "chromium_playwright/dom_interaction.h"
#include "chromium_playwright/screenshot_capture.h"
#include "chromium_playwright/proactive_scraping.h"
#include "chromium_playwright/storage_integration.h"
#include "chromium_playwright/api_layer.h"

using namespace chromium_playwright;

int main() {
    std::cout << "Chromium Playwright Clone - Basic Usage Example" << std::endl;
    std::cout << "================================================" << std::endl;

    try {
        // Initialize the browser control
        std::cout << "1. Initializing browser control..." << std::endl;
        auto& browser = BrowserControl::GetInstance();
        
        // Create a new browser context
        std::cout << "2. Creating browser context..." << std::endl;
        auto context_id = browser.NewContext();
        auto context = browser.GetContext(context_id);
        
        // Create a new page
        std::cout << "3. Creating new page..." << std::endl;
        auto page_id = context->NewPage();
        auto page = context->GetPage(page_id);
        
        // Navigate to a website
        std::cout << "4. Navigating to example.com..." << std::endl;
        NavigationOptions nav_options;
        nav_options.timeout = std::chrono::milliseconds(10000);
        nav_options.wait_until = "domcontentloaded";
        
        if (page->Goto("https://example.com", nav_options)) {
            std::cout << "   ✓ Successfully navigated to: " << page->Url() << std::endl;
            std::cout << "   ✓ Page title: " << page->Title() << std::endl;
        } else {
            std::cout << "   ✗ Failed to navigate to example.com" << std::endl;
            return 1;
        }
        
        // Take a screenshot
        std::cout << "5. Taking screenshot..." << std::endl;
        auto screenshot_capture = CreateScreenshotCapture();
        ScreenshotOptions screenshot_options;
        screenshot_options.path = "example_page.png";
        screenshot_options.full_page = true;
        
        auto screenshot_result = screenshot_capture->CapturePage(*page, screenshot_options);
        if (screenshot_result.success) {
            std::cout << "   ✓ Screenshot saved to: " << screenshot_result.file_path << std::endl;
        } else {
            std::cout << "   ✗ Failed to take screenshot: " << screenshot_result.error_message << std::endl;
        }
        
        // Interact with DOM elements
        std::cout << "6. Interacting with DOM elements..." << std::endl;
        
        // Find elements using different locator strategies
        auto heading_locator = page->GetByCss("h1");
        if (heading_locator->Exists()) {
            std::cout << "   ✓ Found heading: " << heading_locator->TextContent() << std::endl;
        }
        
        auto link_locator = page->GetByText("More information...");
        if (link_locator->Exists()) {
            std::cout << "   ✓ Found link: " << link_locator->TextContent() << std::endl;
            
            // Click the link
            if (link_locator->Click()) {
                std::cout << "   ✓ Successfully clicked the link" << std::endl;
                
                // Wait for navigation
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                std::cout << "   ✓ New URL: " << page->Url() << std::endl;
            } else {
                std::cout << "   ✗ Failed to click the link" << std::endl;
            }
        }
        
        // Take another screenshot after interaction
        std::cout << "7. Taking screenshot after interaction..." << std::endl;
        screenshot_options.path = "example_page_after_click.png";
        screenshot_result = screenshot_capture->CapturePage(*page, screenshot_options);
        if (screenshot_result.success) {
            std::cout << "   ✓ Screenshot saved to: " << screenshot_result.file_path << std::endl;
        }
        
        // Demonstrate proactive scraping
        std::cout << "8. Demonstrating proactive scraping..." << std::endl;
        auto scraper = CreateScraper();
        
        ScrapingConfig scraping_config;
        scraping_config.start_url = page->Url();
        scraping_config.max_depth = 2;
        scraping_config.click_all_buttons = true;
        scraping_config.follow_all_links = true;
        scraping_config.take_screenshots = true;
        scraping_config.max_pages = 5;
        
        // Set up progress callback
        scraper->SetProgressCallback([](const ScrapingSession& session, const ScrapedPageData& page_data) {
            std::cout << "   → Scraped page: " << page_data.url << " (depth: " << page_data.depth << ")" << std::endl;
        });
        
        // Start scraping
        auto session_id = scraper->StartScraping(scraping_config);
        std::cout << "   ✓ Started scraping session: " << session_id << std::endl;
        
        // Wait for scraping to complete (in a real scenario, you'd monitor the session)
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        
        // Get scraping results
        auto results = scraper->GetScrapingResults(session_id);
        std::cout << "   ✓ Scraped " << results.size() << " pages" << std::endl;
        
        // Demonstrate storage integration
        std::cout << "9. Demonstrating storage integration..." << std::endl;
        auto storage = CreateScreenshotStorage();
        
        // Store the scraped data
        for (const auto& page_data : results) {
            auto storage_id = storage->StoreScreenshot(session_id, page_data.url, 
                                                     screenshot_result.image_data, "png");
            if (!storage_id.empty()) {
                std::cout << "   ✓ Stored data for: " << page_data.url << std::endl;
            }
        }
        
        // Demonstrate API layer
        std::cout << "10. Demonstrating API layer..." << std::endl;
        auto api_layer = APIFactory::CreateAPILayer();
        
        APIConfig api_config;
        api_config.endpoint = "localhost:8080";
        api_config.max_connections = 10;
        
        if (api_layer->Initialize(api_config)) {
            std::cout << "   ✓ API layer initialized successfully" << std::endl;
            
            // Register a simple request handler
            api_layer->RegisterRequestHandler("get_page_info", [page](const APIRequest& request) -> APIResponse {
                APIResponse response;
                response.id = request.id;
                response.success = true;
                response.data["url"] = page->Url();
                response.data["title"] = page->Title();
                return response;
            });
            
            std::cout << "   ✓ Registered request handler for 'get_page_info'" << std::endl;
        } else {
            std::cout << "   ✗ Failed to initialize API layer" << std::endl;
        }
        
        // Cleanup
        std::cout << "11. Cleaning up..." << std::endl;
        scraper->StopScraping(session_id);
        page->Close();
        context->Close();
        browser.Shutdown();
        
        std::cout << "✓ Example completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
