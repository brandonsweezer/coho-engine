#pragma once
#include "InstanceComponent.h"

InstanceComponent::InstanceComponent() {
    prototype = nullptr;
}

InstanceComponent::~InstanceComponent() {
    prototype.reset();
}