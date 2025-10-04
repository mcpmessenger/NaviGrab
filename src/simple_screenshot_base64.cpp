#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
#include <gdiplus.h>
#include <sstream>
#include <iomanip>

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
    
    std::string CaptureToBase64() {
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
        
        return base64;
    }
    
private:
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    
    std::string BitmapToBase64(Bitmap* bitmap) {
        // Create IStream for bitmap
        IStream* stream = nullptr;
        CreateStreamOnHGlobal(NULL, TRUE, &stream);
        
        // Save bitmap as PNG to stream
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        bitmap->Save(stream, &pngClsid, NULL);
        
        // Get stream size
        LARGE_INTEGER li;
        li.QuadPart = 0;
        stream->Seek(li, STREAM_SEEK_END, NULL);
        ULONG streamSize = (ULONG)li.QuadPart;
        stream->Seek(li, STREAM_SEEK_SET, NULL);
        
        // Read stream data
        std::vector<BYTE> buffer(streamSize);
        ULONG bytesRead;
        stream->Read(buffer.data(), streamSize, &bytesRead);
        
        // Convert to base64
        std::string base64 = EncodeBase64(buffer.data(), bytesRead);
        
        stream->Release();
        return base64;
    }
    
    std::string EncodeBase64(const BYTE* data, size_t length) {
        const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (size_t i = 0; i < length; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        
        if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        
        return result;
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

// C API
extern "C" {
    SimpleScreenshot* create_screenshot() {
        return new SimpleScreenshot();
    }
    
    void destroy_screenshot(SimpleScreenshot* screenshot) {
        delete screenshot;
    }
    
    const char* capture_to_base64(SimpleScreenshot* screenshot) {
        if (!screenshot) return nullptr;
        
        std::string base64 = screenshot->CaptureToBase64();
        char* result = new char[base64.length() + 1];
        strcpy_s(result, base64.length() + 1, base64.c_str());
        return result;
    }
    
    void free_base64_string(const char* str) {
        delete[] str;
    }
}


