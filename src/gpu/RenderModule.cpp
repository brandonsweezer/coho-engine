#include "RenderModule.h"
#include "../ResourceLoader.h"
#include "../ecs/Entity.h"
#include "../ecs/components/MeshComponent.h"

#include "../memory/Buffer.h"
#include "../memory/Shader.h"
#include "../resources/pipelines/default.h"
#include "../resources/renderpass/skybox.h"
#include "../resources/renderpass/geometry.h"
#include "../resources/renderpass/terrain.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <webgpu/webgpu.hpp>
#include <sdl2webgpu/sdl2webgpu.h>
using namespace wgpu;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using mat4x4 = glm::mat4x4;
using VertexData = Mesh::VertexData;

const float PI = 3.14159265358979323846f;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

RenderModule::RenderModule(int screenWidth, int screenHeight, std::shared_ptr<wgpu::Device> device, std::shared_ptr<wgpu::Surface> surface) {
    m_device = device;
    m_surface = surface;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    init();
}

RenderModule::~RenderModule() {
    terminate();
}

void RenderModule::addTerrainPipeline(
        std::shared_ptr<TerrainPipeline> pipeline,
        std::shared_ptr<coho::Buffer> vertexBuffer,
        uint32_t vertexCount,
        std::shared_ptr<coho::Buffer> indexBuffer,
        uint32_t indexCount,
        std::shared_ptr<coho::Buffer> uniformBuffer
        ) {
    m_terrainvertexBuffer = vertexBuffer;
    m_terrainvertexCount = vertexCount;
    m_terrainindexBuffer = indexBuffer;
    m_terrainindexCount = indexCount;
    m_terrainPipeline = pipeline;
    m_terrainuniformBuffer = uniformBuffer;
}

void RenderModule::writeModelBuffer(std::vector<DefaultPipeline::ModelData> modelData, int offset = 0) {
    m_device->getQueue().writeBuffer(m_modelBuffer->getBuffer(), offset, modelData.data(), modelData.size() * sizeof(DefaultPipeline::ModelData));
}

void RenderModule::writeMaterialBuffer(std::vector<DefaultPipeline::MaterialData> materialData, int offset = 0) {
    m_device->getQueue().writeBuffer(m_materialBuffer->getBuffer(), offset, materialData.data(), materialData.size() * sizeof(DefaultPipeline::MaterialData));
}

void RenderModule::onFrame(
        std::vector<std::shared_ptr<Entity>> terrainPatches,
        std::vector<std::shared_ptr<Entity>> entities,
        std::shared_ptr<Entity> sky, float time) {
    m_uniformData.time = time;
    m_device->getQueue().writeBuffer(m_uniformBuffer->getBuffer(), offsetof(DefaultPipeline::UniformData, time), &m_uniformData.time, sizeof(DefaultPipeline::UniformData::time));
    m_device->getQueue().writeBuffer(m_terrainuniformBuffer->getBuffer(), 0, &m_uniformData, sizeof(TerrainPipeline::UniformData));
    
    // ~~~
    m_surfaceTextureTexture.release();
    m_surface->getCurrentTexture(&m_surfaceTexture);
    m_surfaceTextureTexture = m_surfaceTexture.texture;
    m_surfaceTextureView.release();
    // todo: fix this workflow. feels wrong to release and recreate every frame
    // might be necessary tho :shrug:
    TextureViewDescriptor surfaceViewDesc;
    surfaceViewDesc.format = m_preferredFormat;
    surfaceViewDesc.dimension = TextureViewDimension::_2D;
    surfaceViewDesc.baseMipLevel = 0;
    surfaceViewDesc.mipLevelCount = 1;
    surfaceViewDesc.arrayLayerCount = 1;
    surfaceViewDesc.baseArrayLayer = 0;
    surfaceViewDesc.aspect = TextureAspect::All;
    m_surfaceTextureView = m_surfaceTextureTexture.createView(surfaceViewDesc);
    // ~~~

    if (!m_surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus::WGPUSurfaceGetCurrentTextureStatus_Success) {
        std::cerr << "Could not get current texture view!" << std::endl;
        std::cerr << "m_surfaceTexture.status: " << m_surfaceTexture.status << std::endl;
    }
    
    skyBoxRenderPass(sky);
    geometryRenderPass(entities);
    terrainRenderPass(terrainPatches);
    m_surface->present();
}

void RenderModule::terrainRenderPass(std::vector<std::shared_ptr<Entity>> patches) {
    TerrainRenderPass::render(*m_device,
        m_surfaceTextureView,
        m_depthTextureView,
        m_terrainPipeline->getRenderPipeline(),
        m_terrainvertexBuffer->getBuffer(),
        m_terrainvertexCount * sizeof(VertexData),
        m_terrainindexBuffer->getBuffer(),
        m_terrainindexCount * sizeof(uint32_t),
        m_terrainPipeline->m_bindGroup,
        patches
    );
}

void RenderModule::skyBoxRenderPass(std::shared_ptr<Entity> sky) {
    SkyboxRenderPass::render(*m_device,
        m_surfaceTextureView,
        m_depthTextureView,
        m_renderPipeline->getRenderPipeline(),
        m_vertexBuffer->getBuffer(), 
        m_vertexCount * sizeof(VertexData),
        m_renderPipeline->m_bindGroup,
        {sky}
    );
}

void RenderModule::geometryRenderPass(std::vector<std::shared_ptr<Entity>> entities) {
    GeometryRenderPass::render(*m_device,
        m_surfaceTextureView,
        m_depthTextureView,
        m_renderPipeline->getRenderPipeline(),
        m_vertexBuffer->getBuffer(), 
        m_vertexCount * sizeof(VertexData),
        m_indexBuffer->getBuffer(),
        m_vertexCount * sizeof(VertexData),
        m_renderPipeline->m_bindGroup,
        entities
    );
}

bool RenderModule::init() {
    if (!initBuffers()) return false;
    
    return true;
}

bool RenderModule::startup() {
    if (!initShaderModule()) return false;
    if (!initRenderPipeline()) return false;
    if (!initSurfaceTexture()) return false;
    if (!initDepthBuffer()) return false;

    return true;
}

void RenderModule::terminate() {
    releaseDepthBuffer();
    releaseSurfaceTexture();
    releaseRenderPipeline();
    releaseBuffers();
    releaseShaderModule();
    releaseDevice();
    releaseWindowAndSurface();
}

void RenderModule::releaseDepthBuffer() {
    m_depthTexture.destroy();
    m_depthTexture.release();
    m_depthTextureView.release();
}

void RenderModule::releaseSurfaceTexture() {
    m_surfaceTextureView.release();
    m_surfaceTextureTexture.release();
}

void RenderModule::releaseRenderPipeline() {
    m_renderPipeline.reset();
    m_terrainPipeline.reset();
}

void RenderModule::releaseBuffers() {
    m_indexBuffer.reset();
    m_terrainindexBuffer.reset();
    m_vertexBuffer.reset();
    m_terrainvertexBuffer.reset();
    m_uniformBuffer.reset();
    m_terrainuniformBuffer.reset();
    m_modelBuffer.reset();
    m_terrainmodelBuffer.reset();
    m_materialBuffer.reset();
    m_terrainmaterialBuffer.reset();

}

void RenderModule::releaseShaderModule() {
    m_shader.reset();
}

void RenderModule::releaseDevice() {
    m_device.reset();
}

void RenderModule::releaseWindowAndSurface() {
    m_surface.reset();
}

// returns the vertex offset of this mesh in the vertex buffer.
int RenderModule::addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData) {
    int vertexOffset = m_vertexCount;
    int offset = m_vertexCount * sizeof(VertexData);
    int newDataSize = (int)(vertexData.size() * sizeof(VertexData));
    m_device->getQueue().writeBuffer(m_vertexBuffer->getBuffer(), offset, vertexData.data(), newDataSize);
    m_vertexCount = (int)(m_vertexCount + vertexData.size());
    return vertexOffset;
}

// returns the index offset of this mesh in the index buffer.
int RenderModule::addMeshToIndexBuffer(std::vector<uint32_t> indexData) {
    int indexOffset = m_indexCount;
    int offset = m_indexCount * sizeof(uint32_t);
    int newDataSize = (int)(indexData.size() * sizeof(uint32_t));
    m_device->getQueue().writeBuffer(m_indexBuffer->getBuffer(), offset, indexData.data(), newDataSize);
    m_indexCount = (int)(m_indexCount + indexData.size());
    return indexOffset;
}

// returns the index of the material in the material buffer
int RenderModule::registerMaterial(std::shared_ptr<coho::Material> material) {
    DefaultPipeline::MaterialData md;
    md.baseColor = material->baseColor;
    md.roughness = material->roughness;
    md.diffuseTextureIndex = 0;
    md.normalTextureIndex = 0;

    if (material->diffuseTexture != nullptr) {
        md.diffuseTextureIndex = registerTexture(material->diffuseTexture, material->name + "(diffuse)", material->diffuseTexture->mipLevels);
    }

    if (material->normalTexture != nullptr) {
        md.normalTextureIndex = registerTexture(material->normalTexture, material->name + "(normal)", material->normalTexture->mipLevels);
    }

    int materialIndex = m_materialCount;
    int offset = m_materialCount * sizeof(DefaultPipeline::MaterialData);
    writeMaterialBuffer(std::vector<DefaultPipeline::MaterialData>{md}, offset);
    m_materialCount += 1;
    return materialIndex;
}

// Auxiliary function for registerTexture
void RenderModule::writeMipMaps(
	wgpu::Texture texture,
	wgpu::Extent3D textureSize,
	uint32_t mipLevelCount,
	std::vector<unsigned char> pixelData
    )
{
	// Arguments telling which part of the texture we upload to
	wgpu::ImageCopyTexture destination;
	destination.texture = texture;
	destination.origin = { 0, 0, 0 };
	destination.aspect = wgpu::TextureAspect::All;

	// Arguments telling how the C++ side pixel memory is laid out
	wgpu::TextureDataLayout source;
	source.offset = 0;

	// Create image data
	wgpu::Extent3D mipLevelSize = textureSize;
	std::vector<unsigned char> previousLevelPixels;
	wgpu::Extent3D previousMipLevelSize;
	for (uint32_t level = 0; level < mipLevelCount; ++level) {
		// Pixel data for the current level
		std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
		if (level == 0) {
			// We cannot really avoid this copy since we need this
			// in previousLevelPixels at the next iteration
			memcpy(pixels.data(), pixelData.data(), pixels.size());
		}
		else {
			// Create mip level data
			for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
				for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
					unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
					// Get the corresponding 4 pixels from the previous level
					unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
					unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
					// Average
					p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
					p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
					p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
					p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
				}
			}
		}

		// Upload data to the GPU texture
		destination.mipLevel = level;
		source.bytesPerRow = 4 * mipLevelSize.width;
		source.rowsPerImage = mipLevelSize.height;
		m_device->getQueue().writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

		previousLevelPixels = std::move(pixels);
		previousMipLevelSize = mipLevelSize;
		mipLevelSize.width /= 2;
		mipLevelSize.height /= 2;
	}
}

// returns the index of the registered textureView
int RenderModule::registerTexture(std::shared_ptr<coho::Texture> texture, std::string name, int mipLevelCount) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.dimension = wgpu::TextureDimension::_2D;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm; // png/jpeg format
    textureDesc.label = name.c_str();
    textureDesc.mipLevelCount = mipLevelCount;
    textureDesc.sampleCount = 1;
    textureDesc.size.width = texture->width;
    textureDesc.size.height = texture->height;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
    wgpu::Texture registeredTexture = m_device->createTexture(textureDesc);
    m_textureArray.push_back(registeredTexture);

    wgpu::TextureViewDescriptor texViewDesc;
    texViewDesc.arrayLayerCount = 1;
    texViewDesc.baseArrayLayer = 0;
    texViewDesc.aspect = wgpu::TextureAspect::All;
    texViewDesc.baseMipLevel = 0;
    texViewDesc.mipLevelCount = mipLevelCount;
    texViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    texViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texViewDesc.label = name.c_str();
    TextureView texture_view = registeredTexture.createView(texViewDesc);
    m_textureViewArray.push_back(texture_view);

    writeMipMaps(registeredTexture, textureDesc.size, textureDesc.mipLevelCount, texture->pixelData);

    texture->bufferIndex = (int)m_textureViewArray.size() - 1;
    return texture->bufferIndex;
}

bool RenderModule::initBuffers() {
    std::cout << "initializing buffers" << std::endl;

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(VertexData); // 1,000,000 vertices
    
    BufferBindingLayout bindingLayout = Default;
    bindingLayout.minBindingSize = sizeof(VertexData);
    m_vertexBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "index buffer";
    bufferDesc.usage = BufferUsage::Index | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(VertexData); // 1,000,000 indices
    bindingLayout.minBindingSize = sizeof(VertexData);
    m_indexBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "model buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(DefaultPipeline::ModelData); // 1,000,000 instances
    bindingLayout.minBindingSize = sizeof(DefaultPipeline::ModelData);
    m_modelBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "material buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 100 * sizeof(DefaultPipeline::MaterialData); // 100 materials
    bindingLayout.minBindingSize = sizeof(DefaultPipeline::MaterialData);
    m_materialBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    bufferDesc.label = "uniform buffer";
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.size = sizeof(DefaultPipeline::UniformData);
    bindingLayout.minBindingSize = sizeof(DefaultPipeline::UniformData);
    m_uniformBuffer = std::make_shared<coho::Buffer>(m_device, bufferDesc, bindingLayout, bufferDesc.size, bufferDesc.label);

    m_uniformData.time = 1.0;
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 1000.0f);

    m_device->getQueue().writeBuffer(m_uniformBuffer->getBuffer(), 0, &m_uniformData, sizeof(DefaultPipeline::UniformData));
    updateViewMatrix();

    return true;
}

void RenderModule::releaseTextures() {

    for (auto texture : m_textureArray) {
        texture.destroy();
        texture.release();
    }
    
    for (auto textureView : m_textureViewArray) {
        textureView.release();
    }
}

bool RenderModule::initRenderPipeline() {
    std::cout << "initializing render pipeline" << std::endl;
    m_renderPipeline = std::make_shared<coho::DefaultPipeline>(
        m_vertexBuffer,
        m_indexBuffer,
        m_uniformBuffer,
        m_modelBuffer,
        m_materialBuffer,
        m_shader,
        m_shader
        );

    std::cout << "running render pipeline init tasks" << std::endl;
    if (!m_renderPipeline->init(*m_device, m_preferredFormat, m_textureViewArray)) {
        std::cout << "failed to init render pipeline!" << std::endl;
        return false;
    }

    std::cout << "running terrain pipeline init tasks" << std::endl;
    if (!m_terrainPipeline->init(*m_device, m_preferredFormat)) {
        std::cout << "failed to init terrain pipeline!" << std::endl;
        return false;
    }

    return true;
}

bool RenderModule::initShaderModule() {
    std::cout << "initializing shader module" << std::endl;
    m_shader = std::make_shared<coho::Shader>(RESOURCE_DIR, "shaders/shader.wgsl", m_device);
    
    if (m_shader->getShaderModule() == nullptr) {
        std::cout << "failed to init shader module" << std::endl;
        return false;
    }

    return true;
}

bool RenderModule::initSurfaceTexture() {
    std::cout << "initializing surface texture" << std::endl;
    SurfaceConfiguration surfaceConfig;
    surfaceConfig.alphaMode = CompositeAlphaMode::Auto;
    surfaceConfig.device = *m_device;
    surfaceConfig.format = m_preferredFormat;
    surfaceConfig.height = m_screenHeight;
    surfaceConfig.width = m_screenWidth;
    surfaceConfig.presentMode = PresentMode::Fifo;
    surfaceConfig.usage = TextureUsage::RenderAttachment;
    surfaceConfig.viewFormatCount = 1;
    surfaceConfig.viewFormats = (WGPUTextureFormat*)&m_preferredFormat;

    m_surface->configure(surfaceConfig);

    std::cout << "creating the surface texture" << std::endl;
    m_surface->getCurrentTexture(&m_surfaceTexture);
    m_surfaceTextureTexture = m_surfaceTexture.texture;

    TextureViewDescriptor surfaceViewDesc;
    surfaceViewDesc.format = m_preferredFormat;
    surfaceViewDesc.dimension = TextureViewDimension::_2D;
    surfaceViewDesc.baseMipLevel = 0;
    surfaceViewDesc.mipLevelCount = 1;
    surfaceViewDesc.arrayLayerCount = 1;
    surfaceViewDesc.baseArrayLayer = 0;
    surfaceViewDesc.aspect = TextureAspect::All;
    m_surfaceTextureView = m_surfaceTextureTexture.createView(surfaceViewDesc);
    return true;
}

bool RenderModule::initDepthBuffer() {
    std::cout << "initializing depth buffer" << std::endl;
    TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = TextureDimension::_2D;
    depthTextureDesc.format = m_depthTextureFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size.height = m_screenHeight;
    depthTextureDesc.size.width = m_screenWidth;
    depthTextureDesc.size.depthOrArrayLayers = 1;
    depthTextureDesc.usage = TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;

    m_depthTexture = m_device->createTexture(depthTextureDesc);

    TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = TextureViewDimension::_2D;
    depthTextureViewDesc.format = m_depthTextureFormat;

    std::cout << "creating depth texture view" << std::endl;
    m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);

    SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = AddressMode::ClampToEdge;
    samplerDesc.addressModeV = AddressMode::ClampToEdge;
    samplerDesc.addressModeW = AddressMode::ClampToEdge;
    samplerDesc.magFilter = FilterMode::Linear;
    samplerDesc.minFilter = FilterMode::Linear;
    samplerDesc.mipmapFilter = MipmapFilterMode::Nearest;
    samplerDesc.lodMaxClamp = 10.0;
    samplerDesc.lodMinClamp = 0.0;
    samplerDesc.maxAnisotropy = 1;
    samplerDesc.compare = CompareFunction::LessEqual;
    m_depthSampler = m_device->createSampler(samplerDesc);
    
    return true;
}

void RenderModule::updateProjectionMatrix() {
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 1000.0f);
    m_device->getQueue().writeBuffer(m_uniformBuffer->getBuffer(), offsetof(DefaultPipeline::UniformData, projection_matrix), &m_uniformData.projection_matrix, sizeof(DefaultPipeline::UniformData::projection_matrix));
}

void RenderModule::updateViewMatrix() {
    m_uniformData.camera_world_position = m_camera.position;
    m_uniformData.view_matrix = glm::lookAt(m_camera.position - m_camera.forward, m_camera.position, m_camera.up);
	m_device->getQueue().writeBuffer(m_uniformBuffer->getBuffer(),
		0,
		&m_uniformData,
		sizeof(DefaultPipeline::UniformData)
	);
}

void RenderModule::resizeWindow(int new_width, int new_height) {
    std::cout << "resizing window" << std::endl;
    m_screenWidth = new_width;
    m_screenHeight = new_height;

    releaseDepthBuffer();
    releaseSurfaceTexture();

    initDepthBuffer();
    initSurfaceTexture();

    updateProjectionMatrix();
}

glm::vec2 RenderModule::getScreenDimensions() {
    return glm::vec2(m_screenWidth, m_screenHeight);
}

SDL_Window* RenderModule::getWindow() {
    return m_window;
}