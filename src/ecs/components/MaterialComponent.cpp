#pragma once
#include "MaterialComponent.h"
#include <memory>

MaterialComponent::MaterialComponent() {
    material = std::make_shared<Material>();
}

MaterialComponent::~MaterialComponent() {
    material.reset();
}