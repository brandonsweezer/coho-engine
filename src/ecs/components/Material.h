#include <memory>

class Material {
public:
    std::shared_ptr<Texture> albedoTexture = nullptr;
    std::shared_ptr<Texture> normalTexture = nullptr;
};