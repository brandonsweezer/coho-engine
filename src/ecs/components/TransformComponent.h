#pragma once
#include "Components.h"
#include "Transform.h"
#include <memory>

class TransformComponent : public Component {
public:
    TransformComponent();
    ~TransformComponent();
    std::shared_ptr<Transform> transform;
};