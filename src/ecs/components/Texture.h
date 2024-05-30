#pragma once
#include <vector>
namespace coho {
    class Texture {
    public:
        std::vector<unsigned char> pixelData;
        int width;
        int height;
        int channels;
        int mipLevels;

        int bufferIndex = -1;
    };
}