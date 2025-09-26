#pragma once

#include <vector>
#include <string>
#include <memory>

namespace chromium_playwright::image_processing {

// Image format enumeration
enum class ImageFormat {
    PNG,
    JPEG,
    WEBP,
    BMP
};

// Image data structure
struct ImageData {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 3; // RGB
    std::vector<uint8_t> pixel_data;
    
    // Constructor
    ImageData(uint32_t w, uint32_t h, uint32_t c = 3) 
        : width(w), height(h), channels(c) {
        pixel_data.resize(w * h * c);
    }
    
    // Get pixel at coordinates
    uint8_t GetPixel(uint32_t x, uint32_t y, uint32_t channel) const {
        if (x >= width || y >= height || channel >= channels) {
            return 0;
        }
        size_t index = (y * width + x) * channels + channel;
        return (index < pixel_data.size()) ? pixel_data[index] : 0;
    }
    
    // Set pixel at coordinates
    void SetPixel(uint32_t x, uint32_t y, uint32_t channel, uint8_t value) {
        if (x >= width || y >= height || channel >= channels) {
            return;
        }
        size_t index = (y * width + x) * channels + channel;
        if (index < pixel_data.size()) {
            pixel_data[index] = value;
        }
    }
    
    // Set RGB pixel
    void SetRGB(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
        SetPixel(x, y, 0, r);
        SetPixel(x, y, 1, g);
        SetPixel(x, y, 2, b);
    }
    
    // Get RGB pixel
    void GetRGB(uint32_t x, uint32_t y, uint8_t& r, uint8_t& g, uint8_t& b) const {
        r = GetPixel(x, y, 0);
        g = GetPixel(x, y, 1);
        b = GetPixel(x, y, 2);
    }
    
    // Fill with color
    void Fill(uint8_t r, uint8_t g, uint8_t b) {
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                SetRGB(x, y, r, g, b);
            }
        }
    }
    
    // Get total size in bytes
    size_t GetSize() const {
        return width * height * channels;
    }
    
    // Check if image is valid
    bool IsValid() const {
        return width > 0 && height > 0 && channels > 0 && 
               pixel_data.size() == GetSize();
    }
};

// Image encoder interface
class ImageEncoder {
public:
    virtual ~ImageEncoder() = default;
    
    // Encode image data to specific format
    virtual std::vector<uint8_t> Encode(const ImageData& image_data) = 0;
    
    // Save image to file
    virtual bool SaveToFile(const ImageData& image_data, const std::string& file_path) = 0;
    
    // Get supported format
    virtual ImageFormat GetFormat() const = 0;
    
    // Get file extension for format
    virtual std::string GetFileExtension() const = 0;
};

// Factory function
std::unique_ptr<ImageEncoder> CreateImageEncoder(ImageFormat format);

// Utility functions
namespace image_utils {
    // Create test image
    ImageData CreateTestImage(uint32_t width, uint32_t height, const std::string& pattern = "gradient");
    
    // Load image from file (basic implementation)
    ImageData LoadImage(const std::string& file_path);
    
    // Save image to file
    bool SaveImage(const ImageData& image_data, const std::string& file_path, ImageFormat format = ImageFormat::PNG);
    
    // Convert between formats
    std::vector<uint8_t> ConvertFormat(const ImageData& image_data, ImageFormat target_format);
    
    // Resize image
    ImageData ResizeImage(const ImageData& source, uint32_t new_width, uint32_t new_height);
    
    // Crop image
    ImageData CropImage(const ImageData& source, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    
    // Apply filter
    ImageData ApplyFilter(const ImageData& source, const std::string& filter_name);
    
    // Get image info
    struct ImageInfo {
        uint32_t width;
        uint32_t height;
        uint32_t channels;
        ImageFormat format;
        size_t file_size;
        std::string color_space;
    };
    
    ImageInfo GetImageInfo(const std::string& file_path);
    ImageInfo GetImageInfo(const ImageData& image_data);
}

} // namespace chromium_playwright::image_processing