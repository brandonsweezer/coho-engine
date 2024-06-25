#pragma once
#include "../ecs/Entity.h"
#include "../ecs/components/Mesh.h"
// todo: this should probably not be tied to renderModule. maybe put it in ecs somewhere
#include "../gpu/RenderModule.h"
#include "../memory/Shader.h"
#include "../memory/Buffer.h"
#include "../memory/Pipeline.h"
#include "../resources/pipelines/terrain.h"
#include "Terrain.h"
#include "TerrainPatch.h"
#include <memory>
#include <vector>
#include <webgpu/webgpu.hpp>

class TerrainManager {
public:
    TerrainManager(std::shared_ptr<wgpu::Device> device, int numLods = 1);
    ~TerrainManager();

    std::shared_ptr<TerrainPipeline> getPipeline() { return m_pipeline; };
    std::shared_ptr<coho::Buffer> getVertexBuffer() { return m_vertexBuffer; };
    std::shared_ptr<coho::Buffer> getUniformBuffer() { return m_uniformBuffer; };
    std::shared_ptr<coho::Buffer> getIndexBuffer() { return m_indexBuffer; };
    uint32_t getVertexCount() { return m_vertexCount; };
    uint32_t getIndexCount() { return m_indexCount; };

    std::shared_ptr<Entity> getTerrainPatch();
    std::vector<std::shared_ptr<Entity>> getTerrainPatches(RenderModule::Camera camera);
private: 
    int addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData);
    int addMeshToIndexBuffer(std::vector<uint32_t> indexData);
    int addPatch(std::shared_ptr<Entity> entity);
    void addPatchToModelBuffer(std::vector<TerrainPipeline::ModelData> modelData, int offset = 0);
    void writeModelBuffer(std::vector<TerrainPipeline::ModelData> modelData, int offset = 0);
    bool initBuffers();
    bool initPipeline();
    bool initShaderModule();

private:
    std::vector<TerrainPatch> m_patchLods;
    std::vector<std::shared_ptr<Entity>> m_entities;

    std::shared_ptr<wgpu::Device> m_device;

    TerrainPipeline::UniformData m_uniformData;
    std::shared_ptr<coho::TerrainPipeline> m_pipeline;

    std::shared_ptr<coho::Shader> m_shader = nullptr;
    std::shared_ptr<coho::Buffer> m_indexBuffer;
    int m_indexCount = 0;
    std::shared_ptr<coho::Buffer> m_vertexBuffer;
    int m_vertexCount = 0;
    std::shared_ptr<coho::Buffer> m_modelBuffer;
    std::shared_ptr<coho::Buffer> m_materialBuffer;
    int m_materialCount = 0;
    std::shared_ptr<coho::Buffer> m_uniformBuffer;

    uint32_t m_nextId = 0;
    uint32_t m_nextModelBufferOffset = 0;

};