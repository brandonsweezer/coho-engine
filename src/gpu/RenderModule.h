#pragma once
#include "../ecs/Entity.h"
#include "../ecs/components/Mesh.h"
#include "../ecs/components/Texture.h"
#include "../ecs/components/Material.h"

#include "../memory/Buffer.h"
#include "../memory/Shader.h"

#include "../resources/pipelines/default.h"
#include "../resources/pipelines/terrain.h"

#include <SDL2/SDL.h>
#include <webgpu/webgpu.hpp>
#include <sdl2webgpu/sdl2webgpu.h>
#include <glm/glm.hpp>
#include <vector>

class RenderModule
{

public:

    RenderModule(int screenWidth, int screenHeight, std::shared_ptr<wgpu::Device> device, std::shared_ptr<wgpu::Surface> surface);
    ~RenderModule();

    bool startup();
    
    bool isRunning();
    void onFrame(
        std::vector<std::shared_ptr<Entity>> terrainPatches,
        std::vector<std::shared_ptr<Entity>> entities,
        std::shared_ptr<Entity> sky,
        float time);
    void writeModelBuffer(std::vector<DefaultPipeline::ModelData> modelData, int offset);
    void writeMaterialBuffer(std::vector<DefaultPipeline::MaterialData> materialData, int offset);
    int addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData);
    int addMeshToIndexBuffer(std::vector<uint32_t> indexData);
    void resizeWindow(int new_width, int new_height);

    void addTerrainPipeline(std::shared_ptr<TerrainPipeline> pipeline,
        std::shared_ptr<coho::Buffer> vertexBuffer,
        uint32_t vertexCount,
        std::shared_ptr<coho::Buffer> indexBuffer,
        uint32_t indexCount,
        std::shared_ptr<coho::Buffer> uniformBuffer
        );

    int registerMaterial(std::shared_ptr<coho::Material> material);
    int registerTexture(std::shared_ptr<coho::Texture> texture, std::string filename, int mipLevelCount = 8);

    struct Camera {
        glm::mat4x4 transform;
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
    };
    Camera m_camera;

    void updateProjectionMatrix();
    void updateViewMatrix();
    glm::vec2 getScreenDimensions();
    SDL_Window* getWindow();

private:
    bool init();
    void terminate();

    void writeMipMaps(
	    wgpu::Texture texture,
	    wgpu::Extent3D textureSize,
	    uint32_t mipLevelCount,
	    std::vector<unsigned char> pixelData
        );

    bool initWindowAndSurface();
    void releaseWindowAndSurface();

    bool initDevice();
    void releaseDevice();

    bool initRenderPipeline();
    void releaseRenderPipeline();

    bool initShaderModule();
    void releaseShaderModule();

    bool initBuffers();
    void releaseBuffers();

    void releaseTextures();

    bool initBindGroups();
    void releaseBindGroups();

    bool initSurfaceTexture();
    void releaseSurfaceTexture();

    bool initDepthBuffer();
    void releaseDepthBuffer();

    void geometryRenderPass(std::vector<std::shared_ptr<Entity>> entities);
    void terrainRenderPass(std::vector<std::shared_ptr<Entity>> patches);
    void skyBoxRenderPass(std::shared_ptr<Entity> sky);

private:
    int m_screenWidth = 720;
    int m_screenHeight = 480;

    bool m_isRunning = true;

    DefaultPipeline::UniformData m_uniformData;

    SDL_Window* m_window = nullptr;
    std::shared_ptr<wgpu::Surface> m_surface;
    wgpu::Texture m_surfaceTextureTexture = nullptr;
    wgpu::TextureView m_surfaceTextureView = nullptr;
    wgpu::SurfaceTexture m_surfaceTexture;
    std::shared_ptr<wgpu::Device> m_device = nullptr;
    std::shared_ptr<coho::Shader> m_shader = nullptr;

    wgpu::TextureFormat m_preferredFormat = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;

    std::shared_ptr<coho::DefaultPipeline> m_renderPipeline;
    std::shared_ptr<coho::TerrainPipeline> m_terrainPipeline;

    std::unique_ptr<wgpu::ErrorCallback> m_deviceErrorCallback;

    std::shared_ptr<coho::Buffer> m_indexBuffer;
    std::shared_ptr<coho::Buffer> m_terrainindexBuffer;
    int m_terrainindexCount = 0;
    int m_indexCount = 0;

    std::shared_ptr<coho::Buffer> m_terrainvertexBuffer;
    std::shared_ptr<coho::Buffer> m_vertexBuffer;
    int m_terrainvertexCount = 0;
    int m_vertexCount = 0;

    std::shared_ptr<coho::Buffer> m_terrainmodelBuffer;
    std::shared_ptr<coho::Buffer> m_modelBuffer;

    std::shared_ptr<coho::Buffer> m_terrainmaterialBuffer;
    std::shared_ptr<coho::Buffer> m_materialBuffer;
    int m_terrainmaterialCount = 0;
    int m_materialCount = 0;

    std::shared_ptr<coho::Buffer> m_terrainuniformBuffer;
    std::shared_ptr<coho::Buffer> m_uniformBuffer;

    // textures
    std::vector<wgpu::Texture> m_textureArray;
    std::vector<wgpu::TextureView> m_textureViewArray;

    wgpu::Sampler m_textureSampler = nullptr;
    wgpu::Sampler m_environmentSampler = nullptr;


    wgpu::Texture m_depthTexture = nullptr;
    wgpu::TextureView m_depthTextureView = nullptr;
    wgpu::Sampler m_depthSampler = nullptr;
};