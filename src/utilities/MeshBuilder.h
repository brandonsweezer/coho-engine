#pragma once
#include "../ecs/components/Mesh.h"
#include <memory>
#include <glm/glm.hpp>

class MeshBuilder {
public:
    static std::shared_ptr<Mesh> createUVSphere(int rows, int columns, float radius, bool inFacing = false);
    static std::shared_ptr<Mesh> createCube(int size, bool inFacing = false);
private:
    static glm::vec3 positionFromSphericalCoords(float radius, float pitch, float heading);
    static glm::vec2 uvFromSphericalCoords(float pitch, float heading);
};