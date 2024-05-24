#pragma once
#include "Components.h"
#include "../Entity.h"
#include <memory>

class InstanceComponent : public Component {
public:
    InstanceComponent();
    ~InstanceComponent();
    std::shared_ptr<Entity> prototype;
};