#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef void SimpleScreenshot;

// C API for simple screenshot capture
SimpleScreenshot* create_screenshot();
void destroy_screenshot(SimpleScreenshot* screenshot);
int capture_full_page(SimpleScreenshot* screenshot, const char* filename);
int capture_window(SimpleScreenshot* screenshot, const char* filename, void* hwnd);

// Base64 capture functions
const char* capture_to_base64(SimpleScreenshot* screenshot);
void free_base64_string(const char* str);

#ifdef __cplusplus
}
#endif
