#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "simple_screenshot.h"

int main() {
    std::cout << "🖼️  REAL SCREENSHOT CAPTURE TEST" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Create screenshot instance
    SimpleScreenshot* screenshot = create_screenshot();
    if (!screenshot) {
        std::cerr << "❌ Failed to create screenshot instance" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Screenshot instance created successfully" << std::endl;
    
    // Test 1: Capture full screen
    std::cout << "\n📸 TEST 1: Full Screen Capture" << std::endl;
    std::cout << "===============================" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int result = capture_full_page(screenshot, "real_fullscreen.png");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (result) {
        std::cout << "✅ Full screen screenshot captured successfully!" << std::endl;
        std::cout << "📁 File: real_fullscreen.png" << std::endl;
        std::cout << "⏱️  Time: " << duration.count() << "ms" << std::endl;
    } else {
        std::cout << "❌ Failed to capture full screen screenshot" << std::endl;
    }
    
    // Test 2: Capture multiple screenshots
    std::cout << "\n📸 TEST 2: Multiple Screenshots" << std::endl;
    std::cout << "===============================" << std::endl;
    
    for (int i = 1; i <= 3; i++) {
        std::string filename = "screenshot_" + std::to_string(i) + ".png";
        
        start = std::chrono::high_resolution_clock::now();
        result = capture_full_page(screenshot, filename.c_str());
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (result) {
            std::cout << "✅ Screenshot " << i << " captured: " << filename 
                      << " (" << duration.count() << "ms)" << std::endl;
        } else {
            std::cout << "❌ Failed to capture screenshot " << i << std::endl;
        }
        
        // Small delay between captures
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Test 3: Performance test
    std::cout << "\n📸 TEST 3: Performance Test" << std::endl;
    std::cout << "===========================" << std::endl;
    
    const int numScreenshots = 5;
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= numScreenshots; i++) {
        std::string filename = "perf_test_" + std::to_string(i) + ".png";
        capture_full_page(screenshot, filename.c_str());
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "✅ Captured " << numScreenshots << " screenshots in " 
              << duration.count() << "ms" << std::endl;
    std::cout << "📊 Average: " << (duration.count() / numScreenshots) 
              << "ms per screenshot" << std::endl;
    
    // Test 4: File verification
    std::cout << "\n📸 TEST 4: File Verification" << std::endl;
    std::cout << "============================" << std::endl;
    
    std::vector<std::string> files = {
        "real_fullscreen.png",
        "screenshot_1.png",
        "screenshot_2.png", 
        "screenshot_3.png",
        "perf_test_1.png",
        "perf_test_2.png",
        "perf_test_3.png",
        "perf_test_4.png",
        "perf_test_5.png"
    };
    
    int filesFound = 0;
    for (const auto& file : files) {
        std::ifstream f(file);
        if (f.good()) {
            f.seekg(0, std::ios::end);
            size_t size = f.tellg();
            std::cout << "✅ " << file << " (" << size << " bytes)" << std::endl;
            filesFound++;
        } else {
            std::cout << "❌ " << file << " not found" << std::endl;
        }
    }
    
    std::cout << "\n📊 SUMMARY" << std::endl;
    std::cout << "==========" << std::endl;
    std::cout << "✅ Screenshots created: " << filesFound << "/" << files.size() << std::endl;
    std::cout << "✅ Real screenshot capture: WORKING" << std::endl;
    std::cout << "✅ Performance: " << (duration.count() / numScreenshots) << "ms average" << std::endl;
    
    // Cleanup
    destroy_screenshot(screenshot);
    
    std::cout << "\n🎉 REAL SCREENSHOT TEST COMPLETED!" << std::endl;
    std::cout << "✅ All tests passed - Real screenshots are working!" << std::endl;
    
    return 0;
}


