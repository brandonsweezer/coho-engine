#include "TerrainManager.h"
#include "Terrain.h"

TerrainManager::TerrainManager() {
    m_terrain = Terrain(RESOURCE_DIR, "earth_heightmap.png", 10, 10, 100, 0.5);
}
TerrainManager::~TerrainManager() {
    m_terrain.~Terrain();
}

std::shared_ptr<Entity> TerrainManager::getTerrainPatch() {
    return m_terrain.getTerrain();
}