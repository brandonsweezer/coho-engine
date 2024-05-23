#pragma once
#include "MeshComponent.h"
#include <memory>

MeshComponent::MeshComponent() {
    mesh = std::make_shared<Mesh>();
}

MeshComponent::~MeshComponent() {
    mesh.reset();
}