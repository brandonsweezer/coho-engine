#pragma once
#include "../renderer/Renderer.h"
#include "EntityManager.h"
#include "components/Components.h"
#include "components/TransformComponent.h"
#include "components/InstanceComponent.h"
#include "components/MeshComponent.h"
#include "components/Mesh.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

EntityManager::EntityManager() {
    
}

EntityManager::~EntityManager() {

}

int EntityManager::addEntity(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer) {
    if (entity->hasComponent<InstanceComponent>()) {
        return addInstance(entity, renderer);
    }

    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    
    m_entities.push_back(entity);

    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);
    std::cout << "nextmodelbuffer offset: " << m_nextModelBufferOffset << std::endl;

    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);

    return id;
}

int EntityManager::addInstance(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer) {
    if (!entity->hasComponent<InstanceComponent>()) {
        std::cout << "ERROR: No instance component found on entity!" << std::endl;
        return -1;
    }
    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;

    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);

    m_nextModelBufferOffset += sizeof(Renderer::ModelData);
    return id;
}

int EntityManager::setSky(std::shared_ptr<Entity> sky, std::shared_ptr<Renderer> renderer) {
    m_sky = sky;

    int id = m_nextId;
    sky->setId(m_nextId);
    m_nextId = m_nextId + 1;

    glm::mat4x4 transform = sky->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);

    std::shared_ptr<Mesh> mesh = sky->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);

    return id;
}

std::shared_ptr<Entity> EntityManager::getSky() {
    return m_sky;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() {
    return m_entities;
}

std::shared_ptr<Entity> EntityManager::getEntityById(int id) {
    return m_entities[id];
}