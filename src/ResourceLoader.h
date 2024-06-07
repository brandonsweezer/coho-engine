#pragma once
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>
#include "ecs/components/Mesh.h"
#include "ecs/components/Texture.h"

#include <string>
#include <vector>
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using VertexData = Mesh::VertexData;

class ResourceLoader {
public:
    struct ImageData {
        unsigned char * data;
        int width;
        int height;
        int channels;
    };

    static std::string loadShaderCode(std::string path);
    
    static coho::Texture loadTexture(
        const std::string& path, 
        const std::string& filename
        );

    static bool loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);
    
    bool loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);

    static char* loadBinaryFile(const std::string& path, const std::string& filename, int& filesize);
    static ImageData* loadImage(const std::string& path, const std::string& filename);

};