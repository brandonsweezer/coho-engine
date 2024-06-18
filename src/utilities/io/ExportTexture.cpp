#include "ExportTexture.h"
#include "../../stb_image_write.h"
#include <string>
#include <vector>

int ExportTexture::exportPng(
        std::string path,
        std::string filename,
        uint32_t width,
        uint32_t height,
        uint32_t channels,
        const float* data
    ) {
    // Calculate the total number of pixels
    size_t totalPixels = width * height * channels;

    // Create a buffer to store the converted image data
    std::vector<unsigned char> imageData(totalPixels);

    // Convert float data to unsigned char
    for (size_t i = 0; i < totalPixels; ++i) {
        float value = data[i];
        // Clamp value to the range [0.0, 1.0]
        value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
        imageData[i] = static_cast<unsigned char>(data[i] * 255.0f);
    }
    return stbi_write_png(std::string(path + "/" + filename).c_str(), width, height, channels, imageData.data(), width * channels);
}


int ExportTexture::exportPng(
        std::string path,
        std::string filename,
        uint32_t width,
        uint32_t height,
        uint32_t channels,
        const unsigned char* data
    ) {
    return stbi_write_png(std::string(path + "/" + filename).c_str(), width, height, channels, data, width * channels);
}