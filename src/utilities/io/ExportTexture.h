#pragma once
#include <string>

class ExportTexture {
public:
    static int exportPng(
        std::string path,
        std::string filename,
        uint32_t width,
        uint32_t height,
        uint32_t channels,
        const float* data
    );
    static int exportPng(
        std::string path,
        std::string filename,
        uint32_t width,
        uint32_t height,
        uint32_t channels,
        const unsigned char* data
    );
private:
};