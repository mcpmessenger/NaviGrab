#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <windows.h>
#include "../include/simple_screenshot.h"

int main() {
    std::cout << "🖼️  SCREENSHOT CAPTURE WITH POPUP DISPLAY" << std::endl;
    std::cout << "=========================================" << std::endl;
    
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
    
    const char* base64Data = capture_to_base64(screenshot);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (base64Data) {
        std::cout << "✅ Screenshot captured successfully!" << std::endl;
        std::cout << "⏱️  Capture time: " << duration.count() << "ms" << std::endl;
        std::cout << "📏 Base64 length: " << strlen(base64Data) << " characters" << std::endl;
        
        // Save base64 to file for web display
        std::ofstream file("screenshot_base64.txt");
        if (file.is_open()) {
            file << "data:image/png;base64," << base64Data;
            file.close();
            std::cout << "💾 Base64 data saved to screenshot_base64.txt" << std::endl;
        }
        
        // Create simple thumbnail popup HTML
        std::ofstream htmlFile("screenshot_thumbnail.html");
        if (htmlFile.is_open()) {
            htmlFile << "<!DOCTYPE html>\n";
            htmlFile << "<html>\n";
            htmlFile << "<head>\n";
            htmlFile << "    <title>Screenshot Thumbnail</title>\n";
            htmlFile << "    <style>\n";
            htmlFile << "        body { margin: 0; padding: 10px; background: #2a2a2a; font-family: Arial, sans-serif; }\n";
            htmlFile << "        .thumbnail { max-width: 400px; max-height: 300px; border: 2px solid #555; border-radius: 8px; cursor: pointer; }\n";
            htmlFile << "        .info { color: #ccc; font-size: 12px; margin-top: 5px; }\n";
            htmlFile << "        .close-btn { position: absolute; top: 5px; right: 10px; background: #ff4444; color: white; border: none; border-radius: 3px; padding: 5px 10px; cursor: pointer; }\n";
            htmlFile << "    </style>\n";
            htmlFile << "</head>\n";
            htmlFile << "<body>\n";
            htmlFile << "    <button class=\"close-btn\" onclick=\"window.close()\">×</button>\n";
            htmlFile << "    <img src=\"data:image/png;base64," << base64Data << "\" class=\"thumbnail\" alt=\"Screenshot\" onclick=\"window.close()\">\n";
            htmlFile << "    <div class=\"info\">📸 " << duration.count() << "ms | " << strlen(base64Data) << " chars</div>\n";
            htmlFile << "</body>\n";
            htmlFile << "</html>\n";
            htmlFile.close();
            std::cout << "🖼️ Thumbnail popup created: screenshot_thumbnail.html" << std::endl;
        }
        
        // Open thumbnail popup in browser
        std::cout << "\n🚀 Opening thumbnail popup..." << std::endl;
        ShellExecuteA(NULL, "open", "screenshot_thumbnail.html", NULL, NULL, SW_SHOW);
        
        free_base64_string(base64Data);
    } else {
        std::cerr << "❌ Failed to capture screenshot" << std::endl;
    }
    
    // Cleanup
    destroy_screenshot(screenshot);
    
    std::cout << "\n✅ Test completed!" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}
