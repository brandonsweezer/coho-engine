#pragma once
#include "Components.h"
#include "Material.h"
#include <memory>

class MaterialComponent : public Component {
public:
    MaterialComponent();
    ~MaterialComponent();
    std::shared_ptr<Material> material;
};