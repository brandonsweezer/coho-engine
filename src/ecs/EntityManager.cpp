#pragma once
#include "EntityManager.h"
#include "components/Components.h"
#include <memory>

EntityManager::EntityManager() {
    // Constructor implementation
}

EntityManager::~EntityManager() {
    // Destructor implementation
}

int EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    int id = m_nextId;
    m_entities[m_nextId] = entity;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;

    return id;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() {
    std::vector<std::shared_ptr<Entity>> entities;
    for (auto& entity: m_entities) {
        entities.push_back(entity.second);
    }
    return entities;
}

std::shared_ptr<Entity> EntityManager::getEntityById(int id) {
    return m_entities[id];
}