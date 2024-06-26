#include "TerrainManager.h"
#include "Terrain.h"
#include "../ecs/components/MeshComponent.h"
#include "../ecs/components/TransformComponent.h"
#include "../ecs/components/InstanceComponent.h"

using namespace wgpu;

TerrainManager::TerrainManager(std::shared_ptr<wgpu::Device> device, int numLods) {
    m_device = device;
    if (!initBuffers()) {
        std::cout << "failed to init terrain buffers" << std::endl;
    }
    if (!initShaderModule()) {
        std::cout << "failed to init terrain shader" << std::endl;
    }
    if (!initPipeline()) {
        std::cout << "failed to init terrain pipeline" << std::endl;
    }
    
    // generate all Lods
    for (int lod = 0; lod <= numLods; ++lod) {
        std::cout << "creating patch for lod: " << lod << std::endl;
        float terrainSize = 100.0;
        TerrainPatch patch = TerrainPatch(terrainSize, terrainSize, lod);

        // register Lods in terrain vertex buffer
        std::shared_ptr<Entity> patchEntity = std::make_shared<Entity>();
        patchEntity->addComponent<TransformComponent>();
        patchEntity->addComponent<MeshComponent>()->mesh = patch.getMesh();
        patch.setPrototype(patchEntity);
        addPatch(patchEntity);
        m_patchLods.push_back(patch);

        std::cout << "creating instances for lod: " << lod << std::endl;
        // 100x100 grid of terrain
        // add each lod 100x100 times 
        for (int z = 0; z < 100; z++) {
            for (int x = 0; x < 100; x++) {
                std::shared_ptr<Entity> entity = std::make_shared<Entity>();
                auto prototype = m_patchLods[lod].getPrototype();
                entity->addComponent<InstanceComponent>()->prototype = prototype;
                entity->addComponent<MeshComponent>()->mesh = prototype->getComponent<MeshComponent>()->mesh;
                entity->addComponent<TransformComponent>()->transform->setPosition(
                    glm::vec3(x * (terrainSize / 2) - terrainSize * 10, 0., z * (terrainSize / 2) - terrainSize * 10)
                );
                addInstance(entity);
            }
        }
        m_patchLods[lod].getPrototype()->instanceCount = 100 * 100;
    }
}

TerrainManager::~TerrainManager() {
    m_pipeline.reset();
    m_device.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_shader.reset();
    m_uniformBuffer.reset();
    m_materialBuffer.reset();
    m_modelBuffer.reset();
}

bool TerrainManager::initPipeline() {
    std::cout << "initializing terrain pipeline" << std::endl;
    m_pipeline = std::make_shared<coho::TerrainPipeline>(
        m_vertexBuffer,
        m_indexBuffer,
        m_uniformBuffer,
        m_modelBuffer,
        m_materialBuffer,
        m_shader,
        m_shader
    );

    return true;
}

int TerrainManager::addPatch(std::shared_ptr<Entity> entity) {
    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    
    m_entities.push_back(entity);

    TerrainPipeline::ModelData modelData;

    // write the transform to the model data
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;

    // write the model buffer
    std::vector<TerrainPipeline::ModelData> mds = { modelData };
    writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(TerrainPipeline::ModelData);

    // write the mesh vertices to the vertex buffer
    std::shared_ptr<Mesh> mesh = entity->getComponent<MeshComponent>()->mesh;
    std::vector<Mesh::VertexData> vds = mesh->m_vertexData;
    int vertexBufferOffset = addMeshToVertexBuffer(vds);
    mesh->setVertexBufferOffset(vertexBufferOffset);
    if (mesh->isIndexed) {
        std::vector<uint32_t> indices = mesh->getIndexData();
        int indexBufferOffset = addMeshToIndexBuffer(indices);
        mesh->setIndexBufferOffset(indexBufferOffset);
    }

    // return the id for this entity
    return id;
}

void TerrainManager::writeModelBuffer(std::vector<TerrainPipeline::ModelData> modelData, int offset) {
    m_device->getQueue().writeBuffer(m_modelBuffer->getBuffer(), offset, modelData.data(), modelData.size() * sizeof(TerrainPipeline::ModelData));
}

bool TerrainManager::initShaderModule() {
    std::cout << "initializing terrain shader" << std::endl;
    m_shader = std::make_shared<coho::Shader>(RESOURCE_DIR, "shaders/terrain.wgsl", m_device);

    return true;
}

bool TerrainManager::initBuffers() {
    std::cout << "initializing terrain buffers" << std::endl;

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = 20000 * sizeof(VertexData); // 20,000 vertices (dont need much, just patches)
    
    BufferBindingLayout bindingLayout = Default;
    bindingLayout.minBindingSize = sizeof(VertexData);
    m_vertexBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "index buffer";
    bufferDesc.usage = BufferUsage::Index | BufferUsage::CopyDst;
    bufferDesc.size = 100000 * sizeof(VertexData); // 100,000 indices
    bindingLayout.minBindingSize = sizeof(VertexData);
    m_indexBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "model buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(TerrainPipeline::ModelData); // 1,000,000 patches
    bindingLayout.minBindingSize = sizeof(TerrainPipeline::ModelData);
    m_modelBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "material buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 10 * sizeof(TerrainPipeline::MaterialData); // 10 materials
    bindingLayout.minBindingSize = sizeof(TerrainPipeline::MaterialData);
    m_materialBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "uniform buffer";
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.size = sizeof(TerrainPipeline::UniformData);
    bindingLayout.minBindingSize = sizeof(TerrainPipeline::UniformData);
    m_uniformBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    m_uniformData.time = 1.0;
    float aspectRatio = (float)16 / (float)9;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 1000.0f);

    m_device->getQueue().writeBuffer(m_uniformBuffer->getBuffer(), 0, &m_uniformData, sizeof(TerrainPipeline::UniformData));


    return true;
}

// returns the vertex offset of this mesh in the vertex buffer.
int TerrainManager::addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData) {
    int vertexOffset = m_vertexCount;
    int offset = m_vertexCount * sizeof(VertexData);
    int newDataSize = (int)(vertexData.size() * sizeof(VertexData));
    m_device->getQueue().writeBuffer(m_vertexBuffer->getBuffer(), offset, vertexData.data(), newDataSize);
    m_vertexCount = (int)(m_vertexCount + vertexData.size());
    return vertexOffset;
}


// returns the index offset of this mesh in the index buffer.
int TerrainManager::addMeshToIndexBuffer(std::vector<uint32_t> indexData) {
    int indexOffset = m_indexCount;
    int offset = m_indexCount * sizeof(uint32_t);
    int newDataSize = (int)(indexData.size() * sizeof(uint32_t));
    m_device->getQueue().writeBuffer(m_indexBuffer->getBuffer(), offset, indexData.data(), newDataSize);
    m_indexCount = (int)(m_indexCount + indexData.size());
    return indexOffset;
}

int TerrainManager::addInstance(std::shared_ptr<Entity> entity) {
    if (!entity->hasComponent<InstanceComponent>()) {
        std::cout << "ERROR: No instance component found on entity!" << std::endl;
        return -1;
    }
    std::shared_ptr<Entity> prototype = entity->getComponent<InstanceComponent>()->prototype;

    int id = m_nextId;
    entity->setId(m_nextId);
    m_nextId = m_nextId + 1;
    m_entities.push_back(entity);

    TerrainPipeline::ModelData modelData;
    glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
    modelData.transform = transform;

    std::vector<TerrainPipeline::ModelData> mds = { modelData };
    writeModelBuffer(mds, m_nextModelBufferOffset);
    m_nextModelBufferOffset += sizeof(TerrainPipeline::ModelData);
    
    return id;
}

std::vector<std::shared_ptr<Entity>> TerrainManager::getTerrainPatches(RenderModule::Camera camera) {
    // todo: frustum culling
    if (false) {
        std::cout << camera.position.x << std::endl;
    }
    // determine view frustum
    // return patches within that frustum based on distance from camera

    std::vector<std::shared_ptr<Entity>> patches;
    // for testing purposes, generate 100x100 grid of patches
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            patches.push_back(m_entities[((m_patchLods.size() - 1) * i * j) +(i * j) + j]);
        }
    }

    return patches;
}