#pragma once
#include "Texture.h"
#include <memory>
#include <glm/glm.hpp>
#include <iostream>
using namespace coho;

namespace coho {
    class Material {
    public:
        std::string name;
        glm::vec3 baseColor;
        float roughness;
        std::shared_ptr<Texture> diffuseTexture;
        std::shared_ptr<Texture> normalTexture;

        int materialIndex = -1;
    };
}