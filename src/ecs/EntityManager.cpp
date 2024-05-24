#pragma once
#include "Renderer.h"
#include "EntityManager.h"
#include "components/Components.h"
#include "components/TransformComponent.h"
#include "components/InstanceComponent.h"
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
    if (entity->hasComponent<InstanceComponent>()) {
        int prototypeId = entity->getComponent<InstanceComponent>()->prototype->getId();
        m_instances[prototypeId].push_back(entity);
        int instanceId =(int)m_instances[prototypeId].size();
        entity->setId(instanceId);

        glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
        Renderer::ModelData modelData;
        modelData.transform = transform;
        std::vector<Renderer::ModelData> mds = { modelData };
        m_renderer->writeModelBuffer(mds, m_nextModelBufferOffset);

        m_nextModelBufferOffset += sizeof(Renderer::ModelData);
        return instanceId;
    }

    int id = m_nextId;
    m_entities.push_back(entity);
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;

    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    std::vector<Renderer::ModelData> mds = { modelData };
    m_renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);

    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = m_renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);
    std::cout << "id" << id << std::endl;

    return id;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() {
    return m_entities;
}

std::shared_ptr<Entity> EntityManager::getEntityById(int id) {
    return m_entities[id];
}