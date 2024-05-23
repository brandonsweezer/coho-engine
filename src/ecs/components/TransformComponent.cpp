#pragma once
#include "TransformComponent.h"
#include <memory>

TransformComponent::TransformComponent() {
    transform = std::make_shared<Transform>();
}

TransformComponent::~TransformComponent() {
    transform.reset();
}



