#pragma once
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>

#include <string>
#include <vector>
using vec3 = glm::vec3;
using vec2 = glm::vec2;

class ResourceLoader {
public:
    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };

    static std::string loadShaderCode(std::string path);
    
    static wgpu::Texture loadTexture(const std::string& path, const std::string& filename, wgpu::Device device);

    static bool loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);
    
    bool loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);

    static bool vertexDataCalculations(std::vector<VertexData>& vertexData);

    static glm::mat3x3 calculateTBN(VertexData v0, VertexData v1, VertexData v2, glm::vec3 expectedN);
};