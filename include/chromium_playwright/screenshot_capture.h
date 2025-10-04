#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include "browser_control.h"
#include "dom_interaction.h"

namespace chromium_playwright {

// Screenshot options
struct ScreenshotOptions {
    std::string path = ""; // File path to save screenshot. Empty means return buffer
    bool full_page = false; // Capture full scrollable page
    std::string type = "png"; // Image type: "png" or "jpeg"
    int quality = 100; // JPEG quality (0-100)
    Rect clip; // Clipping region for screenshot
    bool omit_background = false; // Hide default background for transparency
    std::chrono::milliseconds timeout{30000}; // Timeout for screenshot operation
    std::map<std::string, std::string> metadata; // Additional metadata
};

// Image format enumeration
enum class ImageFormat {
    PNG,
    JPEG,
    WEBP,
    BMP
};

// Image quality settings
struct ImageQuality {
    int jpeg_quality = 90; // 0-100
    int png_compression = 6; // 0-9
    bool lossless = false; // For PNG
};

// Screenshot metadata
struct ScreenshotMetadata {
    std::string url;
    std::string title;
    std::chrono::system_clock::time_point timestamp;
    Rect viewport;
    Rect clip_region;
    ImageFormat format;
    int width;
    int height;
    int channels;
    size_t file_size;
    std::map<std::string, std::string> custom_data;
};

// Screenshot result
struct ScreenshotResult {
    bool success = false;
    std::string error_message;
    std::vector<uint8_t> image_data;
    std::string file_path;
    ScreenshotMetadata metadata;
    std::chrono::milliseconds capture_time;
};

// Screenshot Capture Interface
class ScreenshotCapture {
public:
    virtual ~ScreenshotCapture() = default;

    // Page screenshots
    virtual ScreenshotResult CapturePage(Page& page, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureViewport(Page& page, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureFullPage(Page& page, const ScreenshotOptions& options = {}) = 0;

    // Element screenshots
    virtual ScreenshotResult CaptureElement(ElementHandle& element, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureElement(Page& page, const std::string& selector, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureElements(Page& page, const std::vector<std::string>& selectors, const ScreenshotOptions& options = {}) = 0;

    // Region screenshots
    virtual ScreenshotResult CaptureRegion(Page& page, const Rect& region, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureMultipleRegions(Page& page, const std::vector<Rect>& regions, const ScreenshotOptions& options = {}) = 0;

    // Batch operations
    virtual std::vector<ScreenshotResult> CaptureBatch(Page& page, const std::vector<ScreenshotOptions>& options_list) = 0;
    virtual std::vector<ScreenshotResult> CaptureAllElements(Page& page, const std::string& selector, const ScreenshotOptions& options = {}) = 0;

    // Configuration
    virtual void SetDefaultFormat(ImageFormat format) = 0;
    virtual ImageFormat GetDefaultFormat() const = 0;
    virtual void SetDefaultQuality(const ImageQuality& quality) = 0;
    virtual ImageQuality GetDefaultQuality() const = 0;
    virtual void SetDefaultPath(const std::string& path) = 0;
    virtual std::string GetDefaultPath() const = 0;

    // Tooltip-specific screenshots
    virtual ScreenshotResult CaptureTooltipPreview(const std::string& element_id, const std::string& interaction_type = "hover") = 0;
    virtual ScreenshotResult CaptureElementOnHover(const std::string& element_id, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult CaptureInteractionPreview(const std::string& element_id, const std::string& action, const ScreenshotOptions& options = {}) = 0;

    // Utility methods
    virtual bool IsFormatSupported(ImageFormat format) const = 0;
    virtual std::vector<ImageFormat> GetSupportedFormats() const = 0;
    virtual ScreenshotMetadata ExtractMetadata(const std::vector<uint8_t>& image_data) const = 0;
    virtual bool ValidateImage(const std::vector<uint8_t>& image_data) const = 0;
    virtual std::string GetImageInfo(const std::vector<uint8_t>& image_data) const = 0;
};

// Image Processing Interface
class ImageProcessor {
public:
    virtual ~ImageProcessor() = default;

    // Format conversion
    virtual std::vector<uint8_t> ConvertFormat(const std::vector<uint8_t>& image_data, 
                                             ImageFormat from_format, 
                                             ImageFormat to_format,
                                             const ImageQuality& quality = {}) = 0;

    // Image manipulation
    virtual std::vector<uint8_t> Resize(const std::vector<uint8_t>& image_data, 
                                      int new_width, int new_height,
                                      ImageFormat format = ImageFormat::PNG) = 0;
    virtual std::vector<uint8_t> Crop(const std::vector<uint8_t>& image_data, 
                                    const Rect& region,
                                    ImageFormat format = ImageFormat::PNG) = 0;
    virtual std::vector<uint8_t> Rotate(const std::vector<uint8_t>& image_data, 
                                      double angle_degrees,
                                      ImageFormat format = ImageFormat::PNG) = 0;
    virtual std::vector<uint8_t> Flip(const std::vector<uint8_t>& image_data, 
                                    bool horizontal, bool vertical,
                                    ImageFormat format = ImageFormat::PNG) = 0;

    // Quality adjustment
    virtual std::vector<uint8_t> AdjustQuality(const std::vector<uint8_t>& image_data, 
                                             const ImageQuality& quality,
                                             ImageFormat format = ImageFormat::JPEG) = 0;
    virtual std::vector<uint8_t> Compress(const std::vector<uint8_t>& image_data, 
                                        int compression_level,
                                        ImageFormat format = ImageFormat::PNG) = 0;

    // Effects and filters
    virtual std::vector<uint8_t> ApplyFilter(const std::vector<uint8_t>& image_data, 
                                           const std::string& filter_name,
                                           const std::map<std::string, double>& parameters = {},
                                           ImageFormat format = ImageFormat::PNG) = 0;
    virtual std::vector<uint8_t> AddWatermark(const std::vector<uint8_t>& image_data, 
                                            const std::vector<uint8_t>& watermark_data,
                                            const Rect& position,
                                            double opacity = 1.0,
                                            ImageFormat format = ImageFormat::PNG) = 0;

    // Analysis
    virtual Rect GetImageBounds(const std::vector<uint8_t>& image_data) const = 0;
    virtual int GetImageWidth(const std::vector<uint8_t>& image_data) const = 0;
    virtual int GetImageHeight(const std::vector<uint8_t>& image_data) const = 0;
    virtual int GetImageChannels(const std::vector<uint8_t>& image_data) const = 0;
    virtual ImageFormat DetectFormat(const std::vector<uint8_t>& image_data) const = 0;
    virtual bool IsValidImage(const std::vector<uint8_t>& image_data) const = 0;

    // Comparison
    virtual double CompareImages(const std::vector<uint8_t>& image1, 
                               const std::vector<uint8_t>& image2) const = 0;
    virtual bool ImagesEqual(const std::vector<uint8_t>& image1, 
                           const std::vector<uint8_t>& image2, 
                           double threshold = 0.0) const = 0;
    virtual std::vector<Rect> FindDifferences(const std::vector<uint8_t>& image1, 
                                            const std::vector<uint8_t>& image2,
                                            double threshold = 0.1) const = 0;
};

// Screenshot Storage Interface
class ScreenshotStorage {
public:
    virtual ~ScreenshotStorage() = default;

    // Storage operations
    virtual std::string StoreScreenshot(const ScreenshotResult& result) = 0;
    virtual ScreenshotResult RetrieveScreenshot(const std::string& id) = 0;
    virtual bool DeleteScreenshot(const std::string& id) = 0;
    virtual bool ScreenshotExists(const std::string& id) const = 0;

    // Batch operations
    virtual std::vector<std::string> StoreScreenshots(const std::vector<ScreenshotResult>& results) = 0;
    virtual std::vector<ScreenshotResult> RetrieveScreenshots(const std::vector<std::string>& ids) = 0;
    virtual bool DeleteScreenshots(const std::vector<std::string>& ids) = 0;

    // Query operations
    virtual std::vector<std::string> FindScreenshotsByUrl(const std::string& url) const = 0;
    virtual std::vector<std::string> FindScreenshotsByDateRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const = 0;
    virtual std::vector<std::string> FindScreenshotsByMetadata(
        const std::map<std::string, std::string>& criteria) const = 0;

    // Management
    virtual size_t GetStorageSize() const = 0;
    virtual size_t GetScreenshotCount() const = 0;
    virtual bool CleanupOldScreenshots(const std::chrono::system_clock::time_point& cutoff) = 0;
    virtual bool OptimizeStorage() = 0;
};

// Page extension for screenshot capture
class Page {
public:
    // Screenshot methods
    virtual ScreenshotResult Screenshot(const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult ScreenshotElement(const std::string& selector, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult ScreenshotRegion(const Rect& region, const ScreenshotOptions& options = {}) = 0;
    virtual ScreenshotResult ScreenshotFullPage(const ScreenshotOptions& options = {}) = 0;

    // Batch screenshot methods
    virtual std::vector<ScreenshotResult> ScreenshotAllElements(const std::string& selector, const ScreenshotOptions& options = {}) = 0;
    virtual std::vector<ScreenshotResult> ScreenshotMultipleRegions(const std::vector<Rect>& regions, const ScreenshotOptions& options = {}) = 0;
};

// Forward declaration for Locator (defined in dom_interaction.h)
class Locator;

// Factory functions
std::unique_ptr<ScreenshotCapture> CreateScreenshotCapture();
std::unique_ptr<ImageProcessor> CreateImageProcessor();
std::unique_ptr<ScreenshotStorage> CreateScreenshotStorage();

// Utility functions
namespace screenshot_utils {
    // Format conversion
    std::string FormatToString(ImageFormat format);
    ImageFormat StringToFormat(const std::string& format_str);
    
    // File operations
    bool SaveScreenshot(const ScreenshotResult& result, const std::string& path);
    ScreenshotResult LoadScreenshot(const std::string& path);
    
    // Validation
    bool ValidateOptions(const ScreenshotOptions& options);
    bool ValidateRegion(const Rect& region);
    
    // Metadata
    ScreenshotMetadata CreateMetadata(const std::string& url, const std::string& title, 
                                    const Rect& viewport, ImageFormat format);
}

} // namespace chromium_playwright
