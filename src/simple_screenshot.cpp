#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

class SimpleScreenshot {
public:
    SimpleScreenshot() {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }
    
    ~SimpleScreenshot() {
        GdiplusShutdown(gdiplusToken);
    }
    
    bool CaptureFullPage(const std::string& filename) {
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
        
        // Save as PNG
        bool success = SaveBitmapAsPNG(bitmap, filename, screenWidth, screenHeight);
        
        // Cleanup
        SelectObject(memoryDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryDC);
        ReleaseDC(NULL, screenDC);
        
        return success;
    }
    
    bool CaptureWindow(const std::string& filename, HWND hwnd) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        HDC windowDC = GetDC(hwnd);
        HDC memoryDC = CreateCompatibleDC(windowDC);
        
        HBITMAP bitmap = CreateCompatibleBitmap(windowDC, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);
        
        BitBlt(memoryDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
        
        bool success = SaveBitmapAsPNG(bitmap, filename, width, height);
        
        SelectObject(memoryDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryDC);
        ReleaseDC(hwnd, windowDC);
        
        return success;
    }
    
private:
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    
    bool SaveBitmapAsPNG(HBITMAP bitmap, const std::string& filename, int width, int height) {
        // Convert HBITMAP to Gdiplus::Bitmap
        Bitmap* gdiplusBitmap = new Bitmap(bitmap, NULL);
        
        // Create PNG encoder
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        
        // Convert filename to wide string
        std::wstring wfilename(filename.begin(), filename.end());
        
        // Save as PNG
        Status status = gdiplusBitmap->Save(wfilename.c_str(), &pngClsid, NULL);
        
        delete gdiplusBitmap;
        
        return status == Ok;
    }
    
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
        UINT num = 0;
        UINT size = 0;
        
        ImageCodecInfo* pImageCodecInfo = NULL;
        
        GetImageEncodersSize(&num, &size);
        if (size == 0) return -1;
        
        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL) return -1;
        
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
};

// Simple C API for easy integration
extern "C" {
    SimpleScreenshot* create_screenshot() {
        return new SimpleScreenshot();
    }
    
    void destroy_screenshot(SimpleScreenshot* screenshot) {
        delete screenshot;
    }
    
    int capture_full_page(SimpleScreenshot* screenshot, const char* filename) {
        return screenshot->CaptureFullPage(filename) ? 1 : 0;
    }
    
    int capture_window(SimpleScreenshot* screenshot, const char* filename, HWND hwnd) {
        return screenshot->CaptureWindow(filename, hwnd) ? 1 : 0;
    }
}


