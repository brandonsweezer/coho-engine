#pragma once
#include "../renderer/Renderer.h"
#include "EntityManager.h"
#include "components/Components.h"
#include "components/TransformComponent.h"
#include "components/InstanceComponent.h"
#include "components/MeshComponent.h"
#include "components/Mesh.h"
#include "components/MaterialComponent.h"
#include "components/Material.h"
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
    m_renderableEntities.push_back(entity);

    Renderer::ModelData modelData;

    // write the transform to the model data
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;
    modelData.materialIndex = (uint32_t)-1;
    if (entity->hasComponent<MaterialComponent>()) {
        // write the material to the model data
        std::shared_ptr<Material> material = entity->getComponent<MaterialComponent>()->material;
        if (material->materialIndex == -1) { // we gotta register it!
            addMaterial(material, renderer);
        }
        modelData.materialIndex = material->materialIndex;
    }

    // write the model buffer
    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);

    // write the mesh vertices to the vertex buffer
    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);

    // return the id for this entity
    return id;
}

int EntityManager::addInstance(std::shared_ptr<Entity> entity, std::shared_ptr<Renderer> renderer) {
    if (!entity->hasComponent<InstanceComponent>()) {
        std::cout << "ERROR: No instance component found on entity!" << std::endl;
        return -1;
    }
    std::shared_ptr<Entity> prototype = entity->getComponent<InstanceComponent>()->prototype;

    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(entity);

    Renderer::ModelData modelData;
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;

    if (prototype->hasComponent<MaterialComponent>()) {
        // write the material to the model data
        std::shared_ptr<Material> material = prototype->getComponent<MaterialComponent>()->material;
        if (material->materialIndex == -1) { // we gotta register it!
            material->materialIndex = addMaterial(material, renderer);
        }
        modelData.materialIndex = material->materialIndex;
    }

    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);
    
    return id;
}

int EntityManager::setSky(std::shared_ptr<Entity> sky, std::shared_ptr<Renderer> renderer) {
    int id = m_nextId;
    sky->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(sky);
    m_sky = sky;

    glm::mat4x4 transform = sky->getComponent<TransformComponent>()->transform->getMatrix();
    Renderer::ModelData modelData;
    modelData.transform = transform;
    modelData.isSkybox = 1;
    std::shared_ptr<Material> skymaterial = sky->getComponent<MaterialComponent>()->material;
    if (skymaterial->materialIndex == -1) {
        skymaterial->materialIndex = addMaterial(skymaterial, renderer);
    }
    modelData.materialIndex = skymaterial->materialIndex;

    std::vector<Renderer::ModelData> mds = { modelData };
    renderer->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(Renderer::ModelData);

    std::shared_ptr<Mesh> mesh = sky->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->getVertexData();
    int vertexBufferOffset = renderer->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);

    return id;
}

// registers the material with the renderer, assigns it an id, and returns the id
int EntityManager::addMaterial(std::shared_ptr<Material> material, std::shared_ptr<Renderer> renderer) {
    int materialBufferIndex = renderer->registerMaterial(material);
    m_materials.push_back(material);
    int materialId = (int)m_materials.size() - 1;
    material->materialIndex = materialBufferIndex;
    return materialId;
}

std::shared_ptr<Entity> EntityManager::getSky() {
    return m_sky;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() {
    return m_entities;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getRenderableEntities() {
    return m_renderableEntities;
}

std::shared_ptr<Entity> EntityManager::getEntityById(int id) {
    return m_entities[id];
}