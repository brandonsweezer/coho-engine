#pragma once
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>
#include "ecs/components/Mesh.h"

#include <string>
#include <vector>
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using VertexData = Mesh::VertexData;

class ResourceLoader {
public:
    

    static std::string loadShaderCode(std::string path);
    
    static wgpu::Texture loadTexture(
        const std::string& path, 
        const std::string& filename, 
        wgpu::Device& device, 
        wgpu::TextureView& texture_view,
        int mipLevelCount = 8
        );
    static wgpu::Texture loadTexture(
        const std::string& path, 
        const std::string& filename, 
        wgpu::Device& device,
        int mipLevelCount = 8
        );

    static bool loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);
    
    bool loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);

    static bool vertexDataCalculations(std::vector<VertexData>& vertexData);

    static glm::mat3x3 calculateTBN(VertexData v0, VertexData v1, VertexData v2, glm::vec3 expectedN);
};