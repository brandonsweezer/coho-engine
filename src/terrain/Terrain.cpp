#include "Terrain.h"
#include "../utilities/MeshBuilder.h"
#include "../ResourceLoader.h"
#include "../ecs/Entity.h"
#include "../ecs/components/MeshComponent.h"
#include "../ecs/components/Mesh.h"
#include "../ecs/components/MaterialComponent.h"
#include "../ecs/components/Material.h"
#include "../ecs/components/TransformComponent.h"
#include <string>
#include <assert.h>
#include <glm/glm.hpp>

Terrain::Terrain(std::string path, std::string filename, float width, float height, int resolution, float scale) {
    m_terrain = std::make_shared<Entity>();
    std::cout << "loading from image" << std::endl;
    LoadFromImage(path, filename, width, height, resolution, scale);
}

Terrain::Terrain() {
    m_terrain = std::make_shared<Entity>();
}

Terrain::~Terrain() {
    m_terrain.reset();
}

void Terrain::LoadFromImage(std::string path, std::string filename, float width, float height, int resolution, float scale) {
    std::shared_ptr<Mesh> terrainMesh = MeshBuilder::createPlane(width, height, resolution, resolution);
    ResourceLoader::ImageData* imgData = ResourceLoader::loadImage(path, filename);

    for (auto& vd : terrainMesh->m_vertexData) {
        int yOffset = (vd.uv.y) * ((float)imgData->height - 1);
        int xOffset = (vd.uv.x) * ((float)imgData->width - 1);
        float heightSample = imgData->data[(yOffset * (imgData->width) + xOffset) * imgData->channels] / 255.0;
        vd.position.y = heightSample * scale;
        vd.color = vec3(heightSample);
    }

    m_terrain->addComponent<TransformComponent>();
    auto meshComponent = m_terrain->addComponent<MeshComponent>();
    meshComponent->mesh = terrainMesh;
    auto mat = m_terrain->addComponent<MaterialComponent>();
    mat->material->baseColor = vec3(1.0);
    free(imgData);
}

int Terrain::getHeight() {
    return m_height;
}

int Terrain::getWidth() {
    return m_width;
}

float Terrain::getScale() {
    return m_scale;
}

std::shared_ptr<Entity> Terrain::getTerrain() {
    return m_terrain;
}