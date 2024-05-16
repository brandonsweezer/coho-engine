#pragma once
#include "glm/glm.hpp"

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
        glm::vec2 uv;
    };

    static std::string loadShaderCode(std::string path);
    
    static bool loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);
    
    bool ResourceLoader::loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData);

};
