#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <windows.h>
#include "../include/proactive_scraper_simple.h"

int main() {
    std::cout << "🤖 PROACTIVE WEB SCRAPER TEST" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Create scraper instance
    SimpleProactiveScraper* scraper = create_scraper();
    if (!scraper) {
        std::cerr << "❌ Failed to create scraper instance" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Scraper instance created successfully" << std::endl;
    
    // Test 1: Start scraping session
    std::cout << "\n🚀 TEST 1: Starting Scraping Session" << std::endl;
    std::cout << "====================================" << std::endl;
    
    std::string start_url = "https://example.com";
    int max_depth = 2;
    int max_pages = 5;
    
    std::cout << "📝 Configuration:" << std::endl;
    std::cout << "   Start URL: " << start_url << std::endl;
    std::cout << "   Max Depth: " << max_depth << std::endl;
    std::cout << "   Max Pages: " << max_pages << std::endl;
    
    int session_id = start_scraping(scraper, start_url.c_str(), max_depth, max_pages);
    if (session_id <= 0) {
        std::cerr << "❌ Failed to start scraping session" << std::endl;
        destroy_scraper(scraper);
        return 1;
    }
    
    std::cout << "✅ Scraping session started with ID: " << session_id << std::endl;
    
    // Test 2: Monitor scraping progress
    std::cout << "\n📊 TEST 2: Monitoring Progress" << std::endl;
    std::cout << "=============================" << std::endl;
    
    int max_wait_time = 30; // seconds
    int wait_time = 0;
    
    while (wait_time < max_wait_time) {
        if (!is_scraping_running(scraper, session_id)) {
            std::cout << "✅ Scraping completed!" << std::endl;
            break;
        }
        
        int pages_scraped = get_pages_scraped(scraper, session_id);
        std::cout << "📈 Progress: " << pages_scraped << " pages scraped..." << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        wait_time += 2;
    }
    
    if (wait_time >= max_wait_time) {
        std::cout << "⏰ Timeout reached, stopping scraping..." << std::endl;
        stop_scraping(scraper, session_id);
    }
    
    // Test 3: Check scraping status
    std::cout << "\n📋 TEST 3: Checking Status" << std::endl;
    std::cout << "===========================" << std::endl;
    
    int pages_scraped = get_pages_scraped(scraper, session_id);
    std::cout << "📊 Pages scraped: " << pages_scraped << std::endl;
    
    bool is_running = is_scraping_running(scraper, session_id);
    std::cout << "🔄 Scraping running: " << (is_running ? "Yes" : "No") << std::endl;
    
    // Test 4: Check output files
    std::cout << "\n📁 TEST 4: Checking Output Files" << std::endl;
    std::cout << "=================================" << std::endl;
    
    std::filesystem::path output_dir("./scraped_data");
    if (std::filesystem::exists(output_dir)) {
        std::cout << "✅ Output directory exists: " << output_dir << std::endl;
        
        // Count files
        int screenshot_count = 0;
        int thumbnail_count = 0;
        int metadata_count = 0;
        
        for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find("screenshot_") == 0) {
                    screenshot_count++;
                } else if (filename.find("thumb_") == 0) {
                    thumbnail_count++;
                } else if (filename.find("metadata_") == 0) {
                    metadata_count++;
                }
            }
        }
        
        std::cout << "📊 File Statistics:" << std::endl;
        std::cout << "   Screenshots: " << screenshot_count << std::endl;
        std::cout << "   Thumbnails: " << thumbnail_count << std::endl;
        std::cout << "   Metadata: " << metadata_count << std::endl;
        
        // List some files
        std::cout << "\n📋 Sample Files:" << std::endl;
        int file_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
            if (entry.is_regular_file() && file_count < 5) {
                std::cout << "   - " << entry.path().filename().string() << std::endl;
                file_count++;
            }
        }
    } else {
        std::cout << "❌ Output directory not found: " << output_dir << std::endl;
    }
    
    // Test 5: Web interface instructions
    std::cout << "\n🌐 TEST 5: Web Interface" << std::endl;
    std::cout << "========================" << std::endl;
    
    std::cout << "✅ To view results with thumbnail tooltips:" << std::endl;
    std::cout << "   1. Open 'web_interface/proactive_scraper.html' in your browser" << std::endl;
    std::cout << "   2. Click 'Load Stored Data' to see thumbnails" << std::endl;
    std::cout << "   3. Hover over thumbnails to see tooltips" << std::endl;
    std::cout << "   4. Click thumbnails to view full screenshots" << std::endl;
    
    // Cleanup
    destroy_scraper(scraper);
    
    std::cout << "\n✅ PROACTIVE WEB SCRAPER TEST COMPLETED" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}
