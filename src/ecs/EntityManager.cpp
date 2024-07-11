#pragma once
#include "../gpu/RenderModule.h"
#include "EntityManager.h"
#include "components/Components.h"
#include "components/TransformComponent.h"
#include "components/InstanceComponent.h"
#include "components/MeshComponent.h"
#include "components/Mesh.h"
#include "components/MaterialComponent.h"
#include "components/Material.h"
#include "utilities/MeshBuilder.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

EntityManager::EntityManager() {
    camera = std::make_shared<Entity>();
    camera->addComponent<TransformComponent>();
}

EntityManager::~EntityManager() {
    camera.reset();
}

void EntityManager::addDefaultMaterial(std::shared_ptr<RenderModule> renderModule) {
    std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>();
    defaultTexture->channels = 4;
    defaultTexture->width = 1;
    defaultTexture->height = 1;
    defaultTexture->mipLevels = 1;
    defaultTexture->pixelData = std::vector<unsigned char>(1);

    // register default/blank texture
    // register default/blank material
    std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>();
    defaultMaterial->baseColor = glm::vec3(1.0, 0.0, 1.0);
    defaultMaterial->name = "default material";
    defaultMaterial->diffuseTexture = defaultTexture;
    defaultMaterial->normalTexture = defaultTexture;
    defaultMaterial->roughness = 1.0;

    addMaterial(defaultMaterial, renderModule);
}

int EntityManager::addEntity(std::shared_ptr<Entity> entity, std::shared_ptr<RenderModule> renderModule) {
    if (entity->hasComponent<InstanceComponent>()) {
        return addInstance(entity, renderModule);
    }

    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    
    m_entities.push_back(entity);
    m_renderableEntities.push_back(entity);

    DefaultPipeline::ModelData modelData;
    modelData.materialIndex = 0; // default material

    // write the transform to the model data
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;
    if (entity->hasComponent<MaterialComponent>()) {
        // write the material to the model data
        std::shared_ptr<Material> material = entity->getComponent<MaterialComponent>()->material;
        if (material->materialIndex == -1) { // we gotta register it!
            addMaterial(material, renderModule);
        }
        modelData.materialIndex = material->materialIndex;
    }

    // write the model buffer
    std::vector<DefaultPipeline::ModelData> mds = { modelData };
    renderModule->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(DefaultPipeline::ModelData);

    // write the mesh vertices to the vertex buffer
    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->m_vertexData;
    int vertexBufferOffset = renderModule->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);
    if (mesh->isIndexed) {
        std::vector<uint32_t> indices = mesh->getIndexData();
        int indexBufferOffset = renderModule->addMeshToIndexBuffer(indices);
        mesh->setIndexBufferOffset(indexBufferOffset);
    }

    // return the id for this entity
    return id;
}

// todo: pass array of instances and write to model buffer en-mass 
// these sequential queue operations are killer
int EntityManager::addInstance(std::shared_ptr<Entity> entity, std::shared_ptr<RenderModule> renderModule) {
    if (!entity->hasComponent<InstanceComponent>()) {
        std::cout << "ERROR: No instance component found on entity!" << std::endl;
        return -1;
    }
    std::shared_ptr<Entity> prototype = entity->getComponent<InstanceComponent>()->prototype;

    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(entity);

    DefaultPipeline::ModelData modelData;
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;
    modelData.materialIndex = 0;

    if (prototype->hasComponent<MaterialComponent>()) {
        // write the material to the model data
        std::shared_ptr<Material> material = prototype->getComponent<MaterialComponent>()->material;
        if (material->materialIndex == -1) { // we gotta register it!
            material->materialIndex = addMaterial(material, renderModule);
        }
        modelData.materialIndex = material->materialIndex;
    }

    std::vector<DefaultPipeline::ModelData> mds = { modelData };
    renderModule->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(DefaultPipeline::ModelData);
    
    return id;
}

int EntityManager::setSky(std::shared_ptr<Entity> sky, std::shared_ptr<RenderModule> renderModule) {
    int id = m_nextId;
    sky->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(sky);
    m_sky = sky;

    glm::mat4x4 transform = sky->getComponent<TransformComponent>()->transform->getMatrix();
    DefaultPipeline::ModelData modelData;
    modelData.materialIndex = 0;
    modelData.transform = transform;
    modelData.isSkybox = 1;
    if (sky->hasComponent<MaterialComponent>()) {
        std::shared_ptr<Material> skymaterial = sky->getComponent<MaterialComponent>()->material;
        if (skymaterial->materialIndex == -1) {
            skymaterial->materialIndex = addMaterial(skymaterial, renderModule);
        }
        modelData.materialIndex = skymaterial->materialIndex;
    }

    std::vector<DefaultPipeline::ModelData> mds = { modelData };
    renderModule->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(DefaultPipeline::ModelData);

    std::shared_ptr<Mesh> mesh = sky->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->m_vertexData;
    int vertexBufferOffset = renderModule->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);
    if (mesh->isIndexed) {
        std::cout << "adding indices" << std::endl;
        std::vector<uint32_t> indices = mesh->getIndexData();
        int indexBufferOffset = renderModule->addMeshToIndexBuffer(indices);
        mesh->setIndexBufferOffset(indexBufferOffset);
    }

    return id;
}

int EntityManager::setQuad(std::shared_ptr<Entity> quad, std::shared_ptr<RenderModule> renderModule) {
    int id = m_nextId;
    quad->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(quad);
    m_quad = quad;

    glm::mat4x4 transform = quad->getComponent<TransformComponent>()->transform->getMatrix();
    DefaultPipeline::ModelData modelData;
    modelData.materialIndex = 0;
    modelData.transform = transform;
    modelData.isSkybox = 0;

    std::vector<DefaultPipeline::ModelData> mds = { modelData };
    renderModule->writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(DefaultPipeline::ModelData);

    std::shared_ptr<Mesh> mesh = quad->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->m_vertexData;
    int vertexBufferOffset = renderModule->addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);
    
    std::vector<uint32_t> indices = mesh->getIndexData();
    int indexBufferOffset = renderModule->addMeshToIndexBuffer(indices);
    mesh->setIndexBufferOffset(indexBufferOffset);

    return id;
}

// registers the material with the renderModule, assigns it an id, and returns the id
int EntityManager::addMaterial(std::shared_ptr<Material> material, std::shared_ptr<RenderModule> renderModule) {
    int materialBufferIndex = renderModule->registerMaterial(material);
    m_materials.push_back(material);
    int materialId = (int)m_materials.size() - 1;
    material->materialIndex = materialBufferIndex;
    return materialId;
}

std::shared_ptr<Entity> EntityManager::getSky() {
    return m_sky;
}

std::shared_ptr<Entity> EntityManager::getQuad() {
    return m_quad;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() {
    return m_entities;
}

// todo: frustum culling? (m_camera)
std::vector<std::shared_ptr<Entity>> EntityManager::getRenderableEntities() {
    return m_renderableEntities;
}

std::shared_ptr<Entity> EntityManager::getEntityById(int id) {
    return m_entities[id];
}