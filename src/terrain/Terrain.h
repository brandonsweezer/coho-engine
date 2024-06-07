#pragma once
#include "../ecs/components/Mesh.h"
#include "../ecs/Entity.h"
#include <string>
#include <vector>

class Terrain {
public:
    Terrain(std::string path, std::string filename, float width, float height, int resolution, float scale);
    Terrain();
    ~Terrain();

    std::shared_ptr<Entity> getTerrain();
    int getWidth();
    int getHeight();
    float getScale();

private:
    void Terrain::LoadFromImage(std::string path, std::string filename, float width, float height, int resolution, float scale);

private:
    std::shared_ptr<Entity> m_terrain;
    int m_width = 0;
    int m_height = 0;
    float m_scale = 1;
};