#include "Renderer.h"
#include "../ResourceLoader.h"
#include "../ecs/Entity.h"
#include "../ecs/components/MeshComponent.h"

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

Renderer::Renderer() {
    init();
}

Renderer::~Renderer() {
    terminate();
}

void Renderer::writeModelBuffer(std::vector<ModelData> modelData, int offset = 0) {
    m_queue.writeBuffer(m_modelBuffer, offset, modelData.data(), modelData.size() * sizeof(ModelData));
}

void Renderer::writeMaterialBuffer(std::vector<MaterialData> materialData, int offset = 0) {
    m_queue.writeBuffer(m_materialBuffer, offset, materialData.data(), materialData.size() * sizeof(MaterialData));
}

void Renderer::onFrame(std::vector<std::shared_ptr<Entity>> entities, std::shared_ptr<Entity> sky, float time) {
    m_uniformData.time = time;
    m_queue.writeBuffer(m_uniformBuffer, offsetof(UniformData, time), &m_uniformData.time, sizeof(UniformData::time));
    
    // ~~~
    m_surfaceTextureTexture.release();
    m_surface.getCurrentTexture(&m_surfaceTexture);
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
    m_surface.present();
}

void Renderer::skyBoxRenderPass(std::shared_ptr<Entity> sky) {
    RenderPassColorAttachment colorAttachment;
    colorAttachment.clearValue = { 0.0, 0.0, 0.0 };
    colorAttachment.loadOp = LoadOp::Clear;
    colorAttachment.storeOp = StoreOp::Store;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.view = m_surfaceTextureView;

    RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.depthClearValue = 1.0;
    depthStencilAttachment.depthLoadOp = LoadOp::Load;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = LoadOp::Load;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = true;
    depthStencilAttachment.view = m_depthTextureView;

    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.occlusionQuerySet = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    CommandEncoder commandEncoder = m_device.createCommandEncoder(CommandEncoderDescriptor{});
    RenderPassEncoder renderPassEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    renderPassEncoder.setPipeline(m_renderPipeline);
    renderPassEncoder.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexCount * sizeof(VertexData));
    renderPassEncoder.setBindGroup(0, m_bindGroup, 0, nullptr);

    // draw
    auto mesh = sky->getComponent<MeshComponent>()->mesh;
    renderPassEncoder.draw(mesh->getVertexCount(), 1, mesh->getVertexBufferOffset(), sky->getId());

    renderPassEncoder.end();
    CommandBuffer commandBuffer = commandEncoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commandBuffer);
    commandEncoder.release();
    commandBuffer.release();
    renderPassEncoder.release();
}

void Renderer::geometryRenderPass(std::vector<std::shared_ptr<Entity>> entities) {
    RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.clearValue = { 0.0, 0.0, 0.0 };
    renderPassColorAttachment.loadOp = LoadOp::Load;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.view = m_surfaceTextureView;
    
    RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.depthClearValue = 1.0;
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = false;

    depthStencilAttachment.view = m_depthTextureView;

    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.occlusionQuerySet = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    CommandEncoder commandEncoder = m_device.createCommandEncoder(CommandEncoderDescriptor{});
    RenderPassEncoder renderPassEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    renderPassEncoder.setPipeline(m_renderPipeline);
    renderPassEncoder.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexCount * sizeof(VertexData));
    renderPassEncoder.setBindGroup(0, m_bindGroup, 0, nullptr);

    for (auto entity : entities) {
        auto mesh = entity->getComponent<MeshComponent>()->mesh;
        renderPassEncoder.draw(mesh->getVertexCount(), entity->instanceCount, mesh->getVertexBufferOffset(), entity->getId());
    }

    renderPassEncoder.end();
    CommandBuffer commandBuffer = commandEncoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commandBuffer);
    commandEncoder.release();
    commandBuffer.release();
    renderPassEncoder.release();
}

bool Renderer::init() {
    if (!initWindowAndSurface()) return false;
    if (!initDevice()) return false;
    if (!initBuffers()) return false;
    
    return true;
}

bool Renderer::startup() {
    if (!initShaderModule()) return false;
    if (!initTextures()) return false;
    if (!initBindGroups()) return false;
    if (!initRenderPipeline()) return false;
    if (!initSurfaceTexture()) return false;
    if (!initDepthBuffer()) return false;

    return true;
}

void Renderer::terminate() {
    releaseDepthBuffer();
    releaseSurfaceTexture();
    releaseRenderPipeline();
    releaseBindGroups();
    releaseTextures();
    releaseBuffers();
    releaseShaderModule();
    releaseDevice();
    releaseWindowAndSurface();
}

void Renderer::releaseDepthBuffer() {
    m_depthTexture.destroy();
    m_depthTexture.release();
    m_depthTextureView.release();
}

void Renderer::releaseSurfaceTexture() {
    m_surfaceTextureView.release();
    m_surfaceTextureTexture.release();
}

void Renderer::releaseRenderPipeline() {
    m_renderPipeline.release();
}

void Renderer::releaseBindGroups() {
    m_bindGroupLayout.release();
    m_bindGroup.release();
}

void Renderer::releaseBuffers() {
    m_vertexBuffer.destroy();
    m_vertexBuffer.release();
    m_uniformBuffer.destroy();
    m_uniformBuffer.release();

    m_modelBuffer.destroy();
    m_modelBuffer.release();

    m_materialBuffer.destroy();
    m_materialBuffer.release();

}

void Renderer::releaseShaderModule() {
    m_shaderModule.release();
}

void Renderer::releaseDevice() {
    m_queue.release();
    m_device.release();
}

void Renderer::releaseWindowAndSurface() {
    m_surface.release();
    SDL_RELEASE(m_window);
    m_instance.release();
    SDL_Quit();
}

bool Renderer::initWindowAndSurface() {
    m_instance = createInstance(InstanceDescriptor{});
    if (!m_instance) return false;

    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return false;
    }

    std::cout << "initializing window" << std::endl;
    m_window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screenWidth, m_screenHeight, SDL_WINDOW_RESIZABLE);
    if (!m_window) return false;

    std::cout << "initializing surface" << std::endl;
    m_surface = SDL_GetWGPUSurface(m_instance, m_window);
    if (!m_surface) return false;
    
    return true;
}

bool Renderer::initDevice() {
    std::cout << "initializing adapter" << std::endl;
    RequestAdapterOptions adapterOptions;
    adapterOptions.compatibleSurface = m_surface;
    adapterOptions.forceFallbackAdapter = false;
    adapterOptions.powerPreference = PowerPreference::Undefined;
    Adapter adapter = m_instance.requestAdapter(adapterOptions);
    if (!adapter) return false;

    m_preferredFormat = m_surface.getPreferredFormat(adapter);

    std::cout << "initializing device" << std::endl;
    RequiredLimits requiredLimits;
    requiredLimits.limits.minStorageBufferOffsetAlignment = 64;
    requiredLimits.limits.minUniformBufferOffsetAlignment = 64;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(UniformData);
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxBindingsPerBindGroup = 7;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxVertexAttributes = 7;
    requiredLimits.limits.maxBufferSize = 1000000 * sizeof(ModelData); // 1,000,000 models
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexData);
    requiredLimits.limits.maxInterStageShaderComponents = 18;
    requiredLimits.limits.maxStorageBuffersPerShaderStage = 2;
    requiredLimits.limits.maxStorageBufferBindingSize = 1000000 * sizeof(ModelData); // 1 million objects

    requiredLimits.limits.maxTextureDimension1D = 8192;
    requiredLimits.limits.maxTextureDimension2D = 8192;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    requiredLimits.limits.maxSampledTexturesPerShaderStage = 100;
    requiredLimits.limits.maxSamplersPerShaderStage = 2;

    DeviceDescriptor deviceDesc;
    deviceDesc.defaultQueue = QueueDescriptor{};
    deviceDesc.requiredFeatureCount = 2;
    // special conversion for native features
    std::vector<WGPUFeatureName> reqFeatures{
        (WGPUFeatureName)NativeFeature::TextureBindingArray,
        (WGPUFeatureName)NativeFeature::SampledTextureAndStorageBufferArrayNonUniformIndexing
    };
    deviceDesc.requiredFeatures = reqFeatures.data();
    deviceDesc.requiredLimits = &requiredLimits;
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const * message, void * userdata) {
        std::cout << "Device lost! Reason: " << reason << userdata << "(\n" << message << "\n)" << std::endl;
    };

    m_device = adapter.requestDevice(deviceDesc);
    if (!m_device) return false;
    m_deviceErrorCallback = m_device.setUncapturedErrorCallback([](ErrorType type, char const * message) {
        std::cout << "Device error! " << type << "(\n" << message << "\n)" << std::endl;
    });

    std::cout << "initializing queue" << std::endl;
    m_queue = m_device.getQueue();
    if (!m_queue) return false;

    adapter.release();

    return true;
}

// returns the vertex offset of this mesh in the vertex buffer.
int Renderer::addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData) {
    int vertexOffset = m_vertexCount;
    int offset = m_vertexCount * sizeof(VertexData);
    int newDataSize = (int)(vertexData.size() * sizeof(VertexData));
    m_queue.writeBuffer(m_vertexBuffer, offset, vertexData.data(), newDataSize);
    m_vertexCount = (int)(m_vertexCount + vertexData.size());
    return vertexOffset;
}

// returns the index of the material in the material buffer
int Renderer::registerMaterial(std::shared_ptr<coho::Material> material) {
    MaterialData md;
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
    int offset = m_materialCount * sizeof(MaterialData);
    writeMaterialBuffer(std::vector<MaterialData>{md}, offset);
    m_materialCount += 1;
    return materialIndex;
}

// Auxiliary function for registerTexture
void Renderer::writeMipMaps(
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
		m_queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

		previousLevelPixels = std::move(pixels);
		previousMipLevelSize = mipLevelSize;
		mipLevelSize.width /= 2;
		mipLevelSize.height /= 2;
	}
}

// returns the index of the registered textureView
int Renderer::registerTexture(std::shared_ptr<coho::Texture> texture, std::string name, int mipLevelCount) {
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
    wgpu::Texture registeredTexture = m_device.createTexture(textureDesc);
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

bool Renderer::initBuffers() {
    std::cout << "initializing buffers" << std::endl;

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(VertexData); // 1,000,000 vertices
    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.label = "model buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 1000000 * sizeof(ModelData); // 1,000,000 instances
    m_modelBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.label = "material buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 100 * sizeof(MaterialData); // 100 materials
    m_materialBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.label = "uniform buffer";
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.size = sizeof(UniformData);
    m_uniformBuffer = m_device.createBuffer(bufferDesc);

    m_uniformData.time = 1.0;
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 1000.0f);

    m_uniformData.model_matrix = mat4x4(1.0);

    m_queue.writeBuffer(m_uniformBuffer, 0, &m_uniformData, sizeof(UniformData));
    updateViewMatrix();

    return true;
}

bool Renderer::initTextures() {
    std::cout << "loading textures" << std::endl;

    SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = AddressMode::Repeat;
    samplerDesc.addressModeV = AddressMode::Repeat;
    samplerDesc.addressModeW = AddressMode::Repeat;
    samplerDesc.magFilter = FilterMode::Linear;
    samplerDesc.minFilter = FilterMode::Linear;
    samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
    samplerDesc.lodMaxClamp = 8.0;
    samplerDesc.lodMinClamp = 0.0;
    samplerDesc.maxAnisotropy = 1;
    samplerDesc.compare = CompareFunction::Undefined;
    m_textureSampler = m_device.createSampler(samplerDesc);

    m_environmentSampler = m_device.createSampler(samplerDesc);
    return true;
}

void Renderer::releaseTextures() {
    m_textureSampler.release();

    for (auto texture : m_textureArray) {
        texture.destroy();
        texture.release();
    }
    
    for (auto textureView : m_textureViewArray) {
        textureView.release();
    }
}

bool Renderer::initBindGroups() {
    std::cout << "initializing bind groups" << std::endl;
    std::vector<BindGroupLayoutEntry> bindGroupLayoutEntries(5, Default);
    // uniform layout
    bindGroupLayoutEntries[0].binding = 0;
    bindGroupLayoutEntries[0].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[0].buffer.type = BufferBindingType::Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof(UniformData);

    // array of textures
    BindGroupLayoutEntryExtras textureArrayLayout;
    textureArrayLayout.count = (uint32_t)m_textureViewArray.size();
    textureArrayLayout.chain.sType = (WGPUSType)WGPUSType_BindGroupLayoutEntryExtras;
    textureArrayLayout.chain.next = nullptr;
    bindGroupLayoutEntries[1].binding = 1;
    bindGroupLayoutEntries[1].visibility = ShaderStage::Fragment;
    bindGroupLayoutEntries[1].texture.multisampled = false;
    bindGroupLayoutEntries[1].texture.viewDimension = TextureViewDimension::_2D;
    bindGroupLayoutEntries[1].texture.sampleType = TextureSampleType::Float;
    bindGroupLayoutEntries[1].nextInChain = &textureArrayLayout.chain;

    // texture sampler layout
    bindGroupLayoutEntries[2].binding = 2;
    bindGroupLayoutEntries[2].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[2].sampler.type = SamplerBindingType::Filtering;

    // model buffer layout
    bindGroupLayoutEntries[3].binding = 3;
    bindGroupLayoutEntries[3].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[3].buffer.type = BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[3].buffer.minBindingSize = sizeof(ModelData);

    // material buffer layout
    bindGroupLayoutEntries[4].binding = 4;
    bindGroupLayoutEntries[4].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[4].buffer.type = BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[4].buffer.minBindingSize = sizeof(MaterialData);

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();

    m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    std::vector<BindGroupEntry> bindGroupEntries(5, Default);
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].offset = 0;
    bindGroupEntries[0].buffer = m_uniformBuffer;
    bindGroupEntries[0].size = sizeof(UniformData);

    BindGroupEntryExtras textureArray;
    textureArray.textureViewCount = m_textureViewArray.size();
    textureArray.textureViews = (WGPUTextureView*)m_textureViewArray.data();
    textureArray.chain.sType = (WGPUSType)WGPUSType_BindGroupEntryExtras;
    textureArray.chain.next = nullptr;
    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].offset = 0;
    bindGroupEntries[1].nextInChain = &textureArray.chain;

    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].offset = 0;
    bindGroupEntries[2].sampler = m_textureSampler;

    bindGroupEntries[3].binding = 3;
    bindGroupEntries[3].offset = 0;
    bindGroupEntries[3].buffer = m_modelBuffer;
    bindGroupEntries[3].size = 1000000 * sizeof(ModelData); // 1,000,000 instances

    bindGroupEntries[4].binding = 4;
    bindGroupEntries[4].offset = 0;
    bindGroupEntries[4].buffer = m_materialBuffer;
    bindGroupEntries[4].size = 100 * sizeof(MaterialData); // 100 materials

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.entries = bindGroupEntries.data();
    bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
    bindGroupDesc.layout = m_bindGroupLayout;

    m_bindGroup = m_device.createBindGroup(bindGroupDesc);

    return true;
}

bool Renderer::initRenderPipeline() {
    std::cout << "initializing render pipeline" << std::endl;
    RenderPipelineDescriptor renderPipelineDesc;

    std::vector<VertexAttribute> vertexAttributes(6);
    // position
    vertexAttributes[0].format = VertexFormat::Float32x3;
    vertexAttributes[0].offset = offsetof(VertexData, position);
    vertexAttributes[0].shaderLocation = 0;
    // normal
    vertexAttributes[1].format = VertexFormat::Float32x3;
    vertexAttributes[1].offset = offsetof(VertexData, normal);
    vertexAttributes[1].shaderLocation = 1;
    // color
    vertexAttributes[2].format = VertexFormat::Float32x3;
    vertexAttributes[2].offset = offsetof(VertexData, color);
    vertexAttributes[2].shaderLocation = 2;
    // tangent
    vertexAttributes[3].format = VertexFormat::Float32x3;
    vertexAttributes[3].offset = offsetof(VertexData, tangent);
    vertexAttributes[3].shaderLocation = 3;
    // bitangent 
    vertexAttributes[4].format = VertexFormat::Float32x3;
    vertexAttributes[4].offset = offsetof(VertexData, bitangent);
    vertexAttributes[4].shaderLocation = 4;
    // uv
    vertexAttributes[5].format = VertexFormat::Float32x2;
    vertexAttributes[5].offset = offsetof(VertexData, uv);
    vertexAttributes[5].shaderLocation = 5;

    VertexBufferLayout vertexBufferLayout;
    vertexBufferLayout.arrayStride = sizeof(VertexData);
    vertexBufferLayout.attributes = vertexAttributes.data();
    vertexBufferLayout.attributeCount = (uint32_t)vertexAttributes.size();
    vertexBufferLayout.stepMode = VertexStepMode::Vertex;

    renderPipelineDesc.vertex.bufferCount = 1;
    renderPipelineDesc.vertex.buffers = &vertexBufferLayout;
    renderPipelineDesc.vertex.constantCount = 0;
    renderPipelineDesc.vertex.constants = 0;
    renderPipelineDesc.vertex.entryPoint = "vs_main";
    renderPipelineDesc.vertex.module = m_shaderModule;

    BlendState blendState;
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;

    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;

    ColorTargetState colorTargetState;
    colorTargetState.format = m_preferredFormat;
    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = ColorWriteMask::All;

    FragmentState fragmentState;
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.entryPoint = "fs_main";
    fragmentState.module = m_shaderModule;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;
    renderPipelineDesc.fragment = &fragmentState;

    renderPipelineDesc.primitive.cullMode = CullMode::Back;
    renderPipelineDesc.primitive.frontFace = FrontFace::CCW;
    renderPipelineDesc.primitive.topology = PrimitiveTopology::TriangleList; 
    renderPipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;

    DepthStencilState depthStencilState = Default;
    depthStencilState.depthCompare = CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	depthStencilState.format = m_depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

    renderPipelineDesc.depthStencil = &depthStencilState;

    renderPipelineDesc.multisample.count = 1;
    renderPipelineDesc.multisample.mask = ~0u;
	renderPipelineDesc.multisample.alphaToCoverageEnabled = false;

    
    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;

    renderPipelineDesc.layout = m_device.createPipelineLayout(pipelineLayoutDesc);

    m_renderPipeline = m_device.createRenderPipeline(renderPipelineDesc);
    if (!m_renderPipeline) return false;

    return true;
}

bool Renderer::initShaderModule() {
    std::cout << "initializing shader module" << std::endl;
    ShaderModuleWGSLDescriptor shaderModuleWGSLDesc;
    std::string shaderCode = ResourceLoader::loadShaderCode(RESOURCE_DIR "/shader.wgsl");
    shaderModuleWGSLDesc.code = shaderCode.c_str();
    shaderModuleWGSLDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    shaderModuleWGSLDesc.chain.next = nullptr;
    
    ShaderModuleDescriptor shaderModuleDesc;
    shaderModuleDesc.hintCount = 0;
    shaderModuleDesc.hints = nullptr;
    shaderModuleDesc.nextInChain = &shaderModuleWGSLDesc.chain;
    m_shaderModule = m_device.createShaderModule(shaderModuleDesc);

    return true;
}

bool Renderer::initSurfaceTexture() {
    std::cout << "initializing surface texture" << std::endl;
    SurfaceConfiguration surfaceConfig;
    surfaceConfig.alphaMode = CompositeAlphaMode::Auto;
    surfaceConfig.device = m_device;
    surfaceConfig.format = m_preferredFormat;
    surfaceConfig.height = m_screenHeight;
    surfaceConfig.width = m_screenWidth;
    surfaceConfig.presentMode = PresentMode::Fifo;
    surfaceConfig.usage = TextureUsage::RenderAttachment;
    surfaceConfig.viewFormatCount = 1;
    surfaceConfig.viewFormats = (WGPUTextureFormat*)&m_preferredFormat;

    m_surface.configure(surfaceConfig);

    std::cout << "creating the surface texture" << std::endl;
    m_surface.getCurrentTexture(&m_surfaceTexture);
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

bool Renderer::initDepthBuffer() {
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

    m_depthTexture = m_device.createTexture(depthTextureDesc);

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
    m_depthSampler = m_device.createSampler(samplerDesc);
    
    return true;
}

void Renderer::updateProjectionMatrix() {
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 1000.0f);
    m_queue.writeBuffer(m_uniformBuffer, offsetof(UniformData, projection_matrix), &m_uniformData.projection_matrix, sizeof(UniformData::projection_matrix));
}

void Renderer::updateViewMatrix() {
    // TODO: implement trackball camera controls 
    // arcball implementation
    float cx = cos(m_camera.angles.x);
	float cy = cos(m_camera.angles.y);
	float sx = sin(m_camera.angles.x);
	float sy = sin(m_camera.angles.y);

	vec3 position = vec3(cx * cy, sy, sx * cy) * std::exp(-m_camera.zoom);
    m_uniformData.camera_world_position = position;
	m_uniformData.view_matrix = glm::lookAt(position, vec3(0.0f), vec3(0, 1, 0));
	m_queue.writeBuffer(m_uniformBuffer,
		0,
		&m_uniformData,
		sizeof(UniformData)
	);
}

void Renderer::resizeWindow(int new_width, int new_height) {
    std::cout << "resizing window" << std::endl;
    m_screenWidth = new_width;
    m_screenHeight = new_height;

    releaseDepthBuffer();
    releaseSurfaceTexture();

    initDepthBuffer();
    initSurfaceTexture();

    updateProjectionMatrix();
}