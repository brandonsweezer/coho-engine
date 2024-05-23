#pragma once
#include "Components.h"
#include "Mesh.h"
#include <memory>

class MeshComponent : public Component {
public:
    MeshComponent();
    ~MeshComponent();
    std::shared_ptr<Mesh> mesh;
};