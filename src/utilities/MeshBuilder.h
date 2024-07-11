#pragma once
#include "../ecs/components/Mesh.h"
#include <memory>
#include <glm/glm.hpp>

class MeshBuilder {
public:
    static std::shared_ptr<Mesh> createUVSphere(int rows, int columns, float radius, bool inFacing = false);
    static std::shared_ptr<Mesh> createCube(int size, bool inFacing = false);
    static std::shared_ptr<Mesh> createPlane(float width, float height, uint32_t indicesX, uint32_t indicesY);
    static std::shared_ptr<Mesh> createQuad();
    static std::shared_ptr<Mesh> createTerrainPatch(float width, float height, int lod);
private:
    static glm::vec3 positionFromSphericalCoords(float radius, float pitch, float heading);
    static glm::vec2 uvFromSphericalCoords(float pitch, float heading);
};