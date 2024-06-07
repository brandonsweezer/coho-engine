#pragma once
#include "../ecs/Entity.h"
#include "Terrain.h"
#include <memory>

class TerrainManager {
public:
    TerrainManager();
    ~TerrainManager();

    std::shared_ptr<Entity> getTerrainPatch();
private:
    Terrain m_terrain;

};