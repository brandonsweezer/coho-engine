#pragma once
#include "Entity.h"
#include "components/Material.h"
#include <unordered_map>
#include <vector>
#include <memory>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();
    std::shared_ptr<Entity> getEntityById(int id);
    std::vector<std::shared_ptr<Entity>> getAllEntities();
    std::vector<std::shared_ptr<Entity>> getRenderableEntities();
    int addEntity(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer);
    int addInstance(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer);
    void addDefaultMaterial(std::shared_ptr<Renderer> renderer);
    int addMaterial(std::shared_ptr<Material> material, std::shared_ptr<Renderer> renderer);
    int EntityManager::setSky(std::shared_ptr<Entity> sky, std::shared_ptr<Renderer> renderer);
    std::shared_ptr<Entity> getSky();

    std::shared_ptr<Entity> camera;
private:

    std::vector<std::shared_ptr<Entity>> m_entities;
    std::vector<std::shared_ptr<Entity>> m_renderableEntities;
    std::vector<std::shared_ptr<Material>> m_materials;

    std::unordered_map<int, std::vector<std::shared_ptr<Entity>>> m_instances;
    int m_nextId = 0;
    int m_nextModelBufferOffset = 0;
    int m_nextMaterialBufferOffset = 0;

    std::shared_ptr<Entity> m_sky = nullptr;
};