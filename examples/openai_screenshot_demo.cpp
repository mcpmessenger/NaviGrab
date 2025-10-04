#include <iostream>
#include <string>
#include <windows.h>
#include <gdiplus.h>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winhttp.lib")

using namespace Gdiplus;

class SimpleOpenAIScreenshotDemo {
private:
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    
public:
    SimpleOpenAIScreenshotDemo() {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }
    
    ~SimpleOpenAIScreenshotDemo() {
        GdiplusShutdown(gdiplusToken);
    }
    
    // Capture screenshot using Windows API
    std::string CaptureScreenshot() {
        std::cout << "📸 Capturing screenshot..." << std::endl;
        
        // Get screen dimensions
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Create device context for screen
        HDC screenDC = GetDC(NULL);
        HDC memoryDC = CreateCompatibleDC(screenDC);
        
        // Create bitmap
        HBITMAP bitmap = CreateCompatibleBitmap(screenDC, screenWidth, screenHeight);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);
        
        // Copy screen to bitmap
        BitBlt(memoryDC, 0, 0, screenWidth, screenHeight, screenDC, 0, 0, SRCCOPY);
        
        // Create GDI+ bitmap from HBITMAP
        Bitmap* gdiBitmap = new Bitmap(bitmap, NULL);
        
        // Convert to base64
        std::string base64 = BitmapToBase64(gdiBitmap);
        
        // Cleanup
        delete gdiBitmap;
        SelectObject(memoryDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryDC);
        ReleaseDC(NULL, screenDC);
        
        std::cout << "✅ Screenshot captured: " << base64.length() << " characters" << std::endl;
        return base64;
    }
    
    // Convert bitmap to base64
    std::string BitmapToBase64(Bitmap* bitmap) {
        // Create memory stream
        IStream* stream = nullptr;
        CreateStreamOnHGlobal(nullptr, TRUE, &stream);
        
        // Save as PNG to stream
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        bitmap->Save(stream, &pngClsid, nullptr);
        
        // Get stream size
        STATSTG stat;
        stream->Stat(&stat, STATFLAG_NONAME);
        ULARGE_INTEGER size = stat.cbSize;
        
        // Read stream data
        HGLOBAL hGlobal;
        GetHGlobalFromStream(stream, &hGlobal);
        void* data = GlobalLock(hGlobal);
        
        // Convert to base64
        std::string base64 = Base64Encode(static_cast<const char*>(data), size.LowPart);
        
        // Cleanup
        GlobalUnlock(hGlobal);
        stream->Release();
        
        return base64;
    }
    
    // Simple base64 encoding
    std::string Base64Encode(const char* data, size_t length) {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (size_t i = 0; i < length; ++i) {
            unsigned char c = static_cast<unsigned char>(data[i]);
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (result.size() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
    
    // Get encoder CLSID for PNG
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
        UINT num = 0;
        UINT size = 0;
        
        ImageCodecInfo* pImageCodecInfo = nullptr;
        
        GetImageEncodersSize(&num, &size);
        if (size == 0) return -1;
        
        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == nullptr) return -1;
        
        GetImageEncoders(num, size, pImageCodecInfo);
        
        for (UINT j = 0; j < num; ++j) {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;
            }
        }
        
        free(pImageCodecInfo);
        return -1;
    }
    
    // Simulate OpenAI API call (in real implementation, this would make HTTP request)
    std::string AnalyzeScreenshotWithOpenAI(const std::string& base64Image, const std::string& apiKey) {
        std::cout << "🤖 Simulating OpenAI Vision API analysis..." << std::endl;
        
        if (apiKey.empty()) {
            return "Error: OpenAI API key not provided. Please set your API key to use real analysis.";
        }
        
        // Simulate API processing time
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Mock analysis response
        std::string analysis = R"(Screenshot Analysis Results:

🎯 **Content Overview:**
This appears to be a screenshot of a desktop environment with multiple applications and windows visible.

🔍 **Key Elements Detected:**
- Desktop interface with taskbar at the bottom
- Multiple application windows
- File explorer and browser windows
- System tray icons

💡 **UI/UX Observations:**
- Clean, organized desktop layout
- Multiple applications running simultaneously
- Standard Windows interface elements

⚠️ **Potential Issues:**
- Multiple applications may impact system performance
- Consider organizing desktop for better productivity

📊 **Recommendations:**
1. Close unused applications to improve performance
2. Organize desktop icons for better accessibility
3. Consider using virtual desktops for better organization

This is a simulated analysis. In a real implementation, this would be processed by OpenAI's Vision API.)";
        
        std::cout << "✅ Analysis completed (simulated)" << std::endl;
        return analysis;
    }
    
    // Display analysis results
    void DisplayAnalysisResults(const std::string& analysis) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "🤖 OPENAI SCREENSHOT ANALYSIS RESULTS" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        std::cout << analysis << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }
    
    // Main demo function
    void RunDemo() {
        std::cout << "🚀 NaviGrab OpenAI Screenshot Analysis Demo" << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << std::endl;
        
        // Get OpenAI API key from user
        std::string apiKey;
        std::cout << "🔑 Enter your OpenAI API key (or press Enter for demo mode): ";
        std::getline(std::cin, apiKey);
        
        if (apiKey.empty()) {
            std::cout << "⚠️  Running in demo mode with simulated analysis..." << std::endl;
        } else {
            std::cout << "✅ API key provided, will attempt real OpenAI analysis..." << std::endl;
        }
        
        std::cout << std::endl;
        
        // Capture screenshot
        std::string screenshot = CaptureScreenshot();
        
        if (screenshot.empty()) {
            std::cout << "❌ Failed to capture screenshot" << std::endl;
            return;
        }
        
        // Analyze with OpenAI (simulated)
        std::string analysis = AnalyzeScreenshotWithOpenAI(screenshot, apiKey);
        
        // Display results
        DisplayAnalysisResults(analysis);
        
        std::cout << "\n💡 Next Steps:" << std::endl;
        std::cout << "   1. Implement real HTTP client for OpenAI API" << std::endl;
        std::cout << "   2. Add proper error handling and retry logic" << std::endl;
        std::cout << "   3. Integrate with web interface" << std::endl;
        std::cout << "   4. Add support for different analysis types" << std::endl;
        std::cout << std::endl;
        
        std::cout << "✅ Demo completed successfully!" << std::endl;
    }
};

int main() {
    std::cout << "🤖 NaviGrab OpenAI Screenshot Analysis" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    
    try {
        SimpleOpenAIScreenshotDemo demo;
        demo.RunDemo();
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🛑 Press Enter to exit...";
    std::cin.get();
    
    return 0;
}

