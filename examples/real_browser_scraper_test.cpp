#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <windows.h>
#include "../include/real_browser_scraper.h"

int main() {
    std::cout << "🌐 REAL BROWSER SCRAPER TEST" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Create real scraper instance
    RealBrowserScraper* scraper = create_real_scraper();
    if (!scraper) {
        std::cerr << "❌ Failed to create real scraper instance" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Real scraper instance created successfully" << std::endl;
    
    // Test 1: Start scraping real websites
    std::cout << "\n🚀 TEST 1: Starting Real Website Scraping" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    std::string start_url = "https://news.ycombinator.com";
    int max_depth = 2;
    int max_pages = 3;
    
    std::cout << "📝 Configuration:" << std::endl;
    std::cout << "   Start URL: " << start_url << std::endl;
    std::cout << "   Max Depth: " << max_depth << std::endl;
    std::cout << "   Max Pages: " << max_pages << std::endl;
    std::cout << "\n⚠️  WARNING: This will open real websites in your browser!" << std::endl;
    std::cout << "   Make sure you have a stable internet connection." << std::endl;
    std::cout << "   Press Enter to continue..." << std::endl;
    std::cin.get();
    
    int session_id = start_real_scraping(scraper, start_url.c_str(), max_depth, max_pages);
    if (session_id <= 0) {
        std::cerr << "❌ Failed to start real scraping session" << std::endl;
        destroy_real_scraper(scraper);
        return 1;
    }
    
    std::cout << "✅ Real scraping session started with ID: " << session_id << std::endl;
    
    // Test 2: Monitor scraping progress
    std::cout << "\n📊 TEST 2: Monitoring Real Progress" << std::endl;
    std::cout << "===================================" << std::endl;
    
    int max_wait_time = 60; // seconds
    int wait_time = 0;
    
    while (wait_time < max_wait_time) {
        if (!is_real_scraping_running(scraper, session_id)) {
            std::cout << "✅ Real scraping completed!" << std::endl;
            break;
        }
        
        int pages_scraped = get_real_pages_scraped(scraper, session_id);
        std::cout << "📈 Progress: " << pages_scraped << " pages scraped..." << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
        wait_time += 5;
    }
    
    if (wait_time >= max_wait_time) {
        std::cout << "⏰ Timeout reached, stopping real scraping..." << std::endl;
        stop_real_scraping(scraper, session_id);
    }
    
    // Test 3: Check results
    std::cout << "\n📋 TEST 3: Checking Real Results" << std::endl;
    std::cout << "=================================" << std::endl;
    
    int pages_scraped = get_real_pages_scraped(scraper, session_id);
    std::cout << "📊 Pages scraped: " << pages_scraped << std::endl;
    
    bool is_running = is_real_scraping_running(scraper, session_id);
    std::cout << "🔄 Scraping running: " << (is_running ? "Yes" : "No") << std::endl;
    
    // Test 4: Check output files
    std::cout << "\n📁 TEST 4: Checking Real Output Files" << std::endl;
    std::cout << "=====================================" << std::endl;
    
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
    std::cout << "\n🌐 TEST 5: View Real Results" << std::endl;
    std::cout << "===========================" << std::endl;
    
    std::cout << "✅ To view real screenshots with tooltips:" << std::endl;
    std::cout << "   1. Open 'web_interface/proactive_scraper.html' in your browser" << std::endl;
    std::cout << "   2. Click 'Load Stored Data' to see real screenshots" << std::endl;
    std::cout << "   3. Hover over thumbnails to see tooltips" << std::endl;
    std::cout << "   4. Click thumbnails to view full screenshots" << std::endl;
    std::cout << "   5. The screenshots will show the actual websites that were scraped!" << std::endl;
    
    // Cleanup
    destroy_real_scraper(scraper);
    
    std::cout << "\n✅ REAL BROWSER SCRAPER TEST COMPLETED" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}
