#pragma once
#include "../ecs/components/Mesh.h"
#include "../utilities/MeshBuilder.h"
#include <vector>
#include <glm/glm.hpp>

class TerrainPatch {
public:
    TerrainPatch(float width, float height, int LOD) {
        m_lod = LOD;
        m_mesh = MeshBuilder::createTerrainPatch(width, height, LOD);
    };

    ~TerrainPatch() {
        m_mesh.reset();
        m_prototype.reset();
    };

    std::shared_ptr<Mesh> getMesh() {
        return m_mesh;
    };

    int getLOD() {
        return m_lod;
    }

    std::shared_ptr<Entity> getPrototype() {
        return m_prototype;
    }

    void setPrototype(std::shared_ptr<Entity> prototype) {
        m_prototype = prototype;
    }

private:
    int m_lod = 0;
    std::shared_ptr<Entity> m_prototype;

private:
    std::shared_ptr<Mesh> m_mesh;

};