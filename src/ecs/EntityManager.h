#pragma once
#include "Entity.h"
#include <unordered_map>
#include <vector>
#include <memory>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();
    std::shared_ptr<Entity> getEntityById(int id);
    std::vector<std::shared_ptr<Entity>> getAllEntities();
    int addEntity(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer);

private:
    std::vector<std::shared_ptr<Entity>> m_entities;

    std::unordered_map<int, std::vector<std::shared_ptr<Entity>>> m_instances;
    int m_nextId = 0;

    int m_nextModelBufferOffset = 0;

};