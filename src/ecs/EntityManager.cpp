#pragma once
#include "Renderer.h"
#include "EntityManager.h"
#include "components/Components.h"
#include "components/TransformComponent.h"
#include "components/MeshComponent.h"
#include "components/Mesh.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

EntityManager::EntityManager(std::shared_ptr<Renderer> renderer) {
    m_renderer = renderer;
}

EntityManager::~EntityManager() {
    m_renderer.reset();
}

int EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    int id = m_nextId;
    m_entities[m_nextId] = entity;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;

    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    std::vector<Renderer::ModelData> mds = { modelData };
    m_renderer->writeModelBuffer(mds, sizeof(Renderer::ModelData) * id);

    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = m_renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);

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