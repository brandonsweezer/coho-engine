#pragma once
#include "Entity.h"
#include <unordered_map>
#include <vector>
#include <memory>

class EntityManager {
public:
    EntityManager(std::shared_ptr<Renderer> renderer);
    ~EntityManager();
    std::shared_ptr<Entity> getEntityById(int id);
    std::vector<std::shared_ptr<Entity>> getAllEntities();
    int addEntity(std::shared_ptr<Entity> entity);

private:
    std::shared_ptr<Renderer> m_renderer;
    std::unordered_map<int, std::shared_ptr<Entity>> m_entities;
    int m_nextId = 0;
};