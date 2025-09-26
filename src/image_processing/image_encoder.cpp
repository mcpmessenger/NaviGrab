#include "image_encoder.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

namespace chromium_playwright::image_processing {

// PNG Encoder Implementation
class PNGEncoder : public ImageEncoder {
public:
    std::vector<uint8_t> Encode(const ImageData& image_data) override {
        std::vector<uint8_t> png_data;
        
        // PNG Signature
        const uint8_t png_signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        png_data.insert(png_data.end(), png_signature, png_signature + 8);
        
        // IHDR Chunk
        IHDRChunk ihdr;
        ihdr.width = image_data.width;
        ihdr.height = image_data.height;
        ihdr.bit_depth = 8;
        ihdr.color_type = 2; // RGB
        ihdr.compression = 0;
        ihdr.filter = 0;
        ihdr.interlace = 0;
        
        uint32_t crc = CalculateCRC(reinterpret_cast<uint8_t*>(&ihdr), 13);
        ihdr.crc = crc;
        
        // Write IHDR chunk
        WriteChunk(png_data, "IHDR", reinterpret_cast<uint8_t*>(&ihdr), 13);
        
        // IDAT Chunk (compressed image data)
        std::vector<uint8_t> compressed_data = CompressImageData(image_data);
        WriteChunk(png_data, "IDAT", compressed_data.data(), compressed_data.size());
        
        // IEND Chunk
        WriteChunk(png_data, "IEND", nullptr, 0);
        
        return png_data;
    }
    
    bool SaveToFile(const ImageData& image_data, const std::string& file_path) override {
        auto png_data = Encode(image_data);
        
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
        file.close();
        
        return true;
    }

private:
    struct IHDRChunk {
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        uint8_t compression;
        uint8_t filter;
        uint8_t interlace;
        uint32_t crc;
    };
    
    void WriteChunk(std::vector<uint8_t>& png_data, const char* type, const uint8_t* data, size_t length) {
        // Length
        uint32_t length_be = SwapEndian(static_cast<uint32_t>(length));
        png_data.insert(png_data.end(), reinterpret_cast<uint8_t*>(&length_be), 
                       reinterpret_cast<uint8_t*>(&length_be) + 4);
        
        // Type
        png_data.insert(png_data.end(), type, type + 4);
        
        // Data
        if (data && length > 0) {
            png_data.insert(png_data.end(), data, data + length);
        }
        
        // CRC
        uint32_t crc = CalculateCRC(reinterpret_cast<const uint8_t*>(type), 4);
        if (data && length > 0) {
            crc = CalculateCRC(data, length, crc);
        }
        uint32_t crc_be = SwapEndian(crc);
        png_data.insert(png_data.end(), reinterpret_cast<uint8_t*>(&crc_be), 
                       reinterpret_cast<uint8_t*>(&crc_be) + 4);
    }
    
    std::vector<uint8_t> CompressImageData(const ImageData& image_data) {
        // Simple compression simulation (in real implementation, use zlib)
        std::vector<uint8_t> compressed;
        
        // Add filter type (0 = None)
        for (size_t y = 0; y < image_data.height; ++y) {
            compressed.push_back(0); // Filter type
            
            for (size_t x = 0; x < image_data.width; ++x) {
                size_t index = (y * image_data.width + x) * 3;
                if (index + 2 < image_data.pixel_data.size()) {
                    compressed.push_back(image_data.pixel_data[index]);     // R
                    compressed.push_back(image_data.pixel_data[index + 1]); // G
                    compressed.push_back(image_data.pixel_data[index + 2]); // B
                }
            }
        }
        
        return compressed;
    }
    
    uint32_t CalculateCRC(const uint8_t* data, size_t length, uint32_t crc = 0xffffffff) {
        static const uint32_t crc_table[256] = {
            0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
            0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
            0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d77, 0x1db71064, 0x6ab020f2,
            0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
            0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
            0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
            0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
            0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
            0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
            0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
            0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
            0x98d2202e, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
            0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
            0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
            0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
            0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
            0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
            0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
            0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
            0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
            0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
            0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
            0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
            0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
            0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
            0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
            0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
            0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
            0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
            0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
            0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
            0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
            0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
            0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
            0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
            0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
            0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
            0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
            0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
            0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
            0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
            0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
            0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
        };
        
        for (size_t i = 0; i < length; ++i) {
            crc = crc_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
        }
        
        return crc ^ 0xffffffff;
    }
    
    uint32_t SwapEndian(uint32_t value) {
        return ((value & 0xff000000) >> 24) |
               ((value & 0x00ff0000) >> 8) |
               ((value & 0x0000ff00) << 8) |
               ((value & 0x000000ff) << 24);
    }
};

// JPEG Encoder Implementation
class JPEGEncoder : public ImageEncoder {
public:
    std::vector<uint8_t> Encode(const ImageData& image_data) override {
        std::vector<uint8_t> jpeg_data;
        
        // JPEG Start of Image marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xD8);
        
        // Quantization Table (simplified)
        WriteQuantizationTable(jpeg_data);
        
        // Start of Frame
        WriteStartOfFrame(jpeg_data, image_data.width, image_data.height);
        
        // Huffman Tables
        WriteHuffmanTables(jpeg_data);
        
        // Start of Scan
        WriteStartOfScan(jpeg_data);
        
        // Image Data (simplified - in real implementation, use proper JPEG compression)
        WriteImageData(jpeg_data, image_data);
        
        // End of Image marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xD9);
        
        return jpeg_data;
    }
    
    bool SaveToFile(const ImageData& image_data, const std::string& file_path) override {
        auto jpeg_data = Encode(image_data);
        
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(jpeg_data.data()), jpeg_data.size());
        file.close();
        
        return true;
    }

private:
    void WriteQuantizationTable(std::vector<uint8_t>& jpeg_data) {
        // DQT marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xDB);
        
        // Length
        jpeg_data.push_back(0x00);
        jpeg_data.push_back(0x43);
        
        // Table ID and precision
        jpeg_data.push_back(0x00);
        
        // Quantization table (simplified)
        for (int i = 0; i < 64; ++i) {
            jpeg_data.push_back(16); // Simple quantization
        }
    }
    
    void WriteStartOfFrame(std::vector<uint8_t>& jpeg_data, uint32_t width, uint32_t height) {
        // SOF0 marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xC0);
        
        // Length
        jpeg_data.push_back(0x00);
        jpeg_data.push_back(0x11);
        
        // Precision
        jpeg_data.push_back(0x08);
        
        // Height
        jpeg_data.push_back((height >> 8) & 0xFF);
        jpeg_data.push_back(height & 0xFF);
        
        // Width
        jpeg_data.push_back((width >> 8) & 0xFF);
        jpeg_data.push_back(width & 0xFF);
        
        // Number of components
        jpeg_data.push_back(0x03);
        
        // Component 1 (Y)
        jpeg_data.push_back(0x01);
        jpeg_data.push_back(0x11);
        jpeg_data.push_back(0x00);
        
        // Component 2 (Cb)
        jpeg_data.push_back(0x02);
        jpeg_data.push_back(0x11);
        jpeg_data.push_back(0x01);
        
        // Component 3 (Cr)
        jpeg_data.push_back(0x03);
        jpeg_data.push_back(0x11);
        jpeg_data.push_back(0x01);
    }
    
    void WriteHuffmanTables(std::vector<uint8_t>& jpeg_data) {
        // DHT marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xC4);
        
        // Length (simplified)
        jpeg_data.push_back(0x00);
        jpeg_data.push_back(0x1F);
        
        // Table class and destination
        jpeg_data.push_back(0x00);
        
        // Huffman table (simplified)
        for (int i = 0; i < 16; ++i) {
            jpeg_data.push_back(0x00);
        }
    }
    
    void WriteStartOfScan(std::vector<uint8_t>& jpeg_data) {
        // SOS marker
        jpeg_data.push_back(0xFF);
        jpeg_data.push_back(0xDA);
        
        // Length
        jpeg_data.push_back(0x00);
        jpeg_data.push_back(0x0C);
        
        // Number of components
        jpeg_data.push_back(0x03);
        
        // Component 1
        jpeg_data.push_back(0x01);
        jpeg_data.push_back(0x00);
        
        // Component 2
        jpeg_data.push_back(0x02);
        jpeg_data.push_back(0x11);
        
        // Component 3
        jpeg_data.push_back(0x03);
        jpeg_data.push_back(0x11);
        
        // Spectral selection
        jpeg_data.push_back(0x00);
        jpeg_data.push_back(0x3F);
        jpeg_data.push_back(0x00);
    }
    
    void WriteImageData(std::vector<uint8_t>& jpeg_data, const ImageData& image_data) {
        // Simplified image data writing
        for (size_t i = 0; i < image_data.pixel_data.size(); i += 3) {
            if (i + 2 < image_data.pixel_data.size()) {
                jpeg_data.push_back(image_data.pixel_data[i]);     // R
                jpeg_data.push_back(image_data.pixel_data[i + 1]); // G
                jpeg_data.push_back(image_data.pixel_data[i + 2]); // B
            }
        }
    }
};

// Factory function
std::unique_ptr<ImageEncoder> CreateImageEncoder(ImageFormat format) {
    switch (format) {
        case ImageFormat::PNG:
            return std::make_unique<PNGEncoder>();
        case ImageFormat::JPEG:
            return std::make_unique<JPEGEncoder>();
        default:
            return std::make_unique<PNGEncoder>();
    }
}

} // namespace chromium_playwright::image_processing
