#include "Renderer.h"
#include "ResourceLoader.h"
#include "ecs/Entity.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/Mesh.h"

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

void Renderer::writeModelBuffer(std::vector<ModelData> modelData, int offset = 0) {
    std::cout << "writing model buffer" << std::endl;
    m_queue.writeBuffer(m_modelBuffer, offset, modelData.data(), modelData.size() * sizeof(ModelData));
}

void Renderer::onFrame(std::vector<std::shared_ptr<Entity>> entities) {
    handleInput();

    m_uniformData.time = SDL_GetTicks64() / 1000.0f;
    m_queue.writeBuffer(m_uniformBuffer, offsetof(UniformData, time), &m_uniformData.time, sizeof(UniformData::time));

    TextureView currentTextureView = m_swapChain.getCurrentTextureView();
    if (!currentTextureView) {
        std::cerr << "Could not get current texture view!" << std::endl;
    }

    RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.clearValue = { 0.0, 0.0, 0.0 };
    renderPassColorAttachment.loadOp = LoadOp::Clear;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.view = currentTextureView;
    
    RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.depthClearValue = 1.0;
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = true;

    depthStencilAttachment.view = m_depthTextureView;

    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.occlusionQuerySet = nullptr;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;

    CommandEncoder commandEncoder = m_device.createCommandEncoder(CommandEncoderDescriptor{});
    RenderPassEncoder renderPassEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    renderPassEncoder.setPipeline(m_renderPipeline);
    renderPassEncoder.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexCount * sizeof(VertexData));
    renderPassEncoder.setBindGroup(0, m_bindGroup, 0, nullptr);
    
    for (auto entity: entities) {
        auto mesh = entity->getComponent<MeshComponent>()->mesh;
        renderPassEncoder.draw(mesh->getVertexCount(), 1, mesh->getVertexBufferOffset(), entity->getId());
    }

    renderPassEncoder.end();
    CommandBuffer commandBuffer = commandEncoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commandBuffer);
    commandBuffer.release();
    renderPassEncoder.release();
    commandEncoder.release();
    
    m_swapChain.present();
    currentTextureView.release();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                m_isRunning = false;
                break;
            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_RESIZED:
                        std::cout << "window resized!!" << std::endl;
                        resizeWindow(e.window.data1, e.window.data2);
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                m_dragState.active = true;
                m_dragState.startCameraState = m_camera;
                int xpos, ypos;
                SDL_GetMouseState(&xpos, &ypos);
                m_dragState.startMouse = vec2((float)xpos, (float)ypos);
                break;
            case SDL_MOUSEBUTTONUP:
                m_dragState.active = false;
                break;
            case SDL_MOUSEWHEEL:
                m_camera.zoom += e.wheel.y * .1f;
                updateViewMatrix();
                break;
            case SDL_KEYDOWN:
                m_keys[e.key.keysym.sym] = true;
                break;
            case SDL_KEYUP:
                m_keys[e.key.keysym.sym] = false;
                break;
            default:
                break;
        }
    }
    
}

void Renderer::handleInput() {
    if (m_dragState.active) {
        int xpos, ypos;
        SDL_GetMouseState(&xpos, &ypos);
        vec2 currentMouse = vec2((float)xpos, (float)ypos);
		vec2 delta = (currentMouse - m_dragState.startMouse) * m_dragState.sensitivity;
		m_camera.angles = m_dragState.startCameraState.angles + delta;
		// Clamp to avoid going too far when orbitting up/down
		m_camera.angles.y = glm::clamp(m_camera.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);

		m_dragState.velocity = delta - m_dragState.previousDelta;
		m_dragState.previousDelta = delta;
        updateViewMatrix();
    }

}

bool Renderer::isRunning() {
    return m_isRunning;
}

bool Renderer::init() {
    m_keys.resize(322, false); // 322 is number of SDL keycodes
    if (!initWindowAndSurface()) return false;
    if (!initDevice()) return false;
    if (!initShaderModule()) return false;
    if (!initBuffers()) return false;
    if (!initTextures()) return false;
    if (!initBindGroups()) return false;
    if (!initRenderPipeline()) return false;
    if (!initSwapChain()) return false;
    if (!initDepthBuffer()) return false;

    return true;
}

void Renderer::terminate() {
    // TODO: implement shared pointers or RAII of some sort
    // still figuring out where/how to implement a wrapper class
    // or if that's unnecessary and i should just use shared_ptrs?
    releaseDepthBuffer();
    releaseSwapChain();
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

void Renderer::releaseSwapChain() {
    m_swapChain.release();
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

    std::cout << "initializing SDL" << std::endl;
    SDL_SetMainReady();
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
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxVertexAttributes = 7;
    requiredLimits.limits.maxBufferSize = 10000000 * sizeof(VertexData); // 10,000,000 verts
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexData);
    requiredLimits.limits.maxInterStageShaderComponents = 17;
    requiredLimits.limits.maxStorageBuffersPerShaderStage = 1;
    requiredLimits.limits.maxStorageBufferBindingSize = 1000000 * sizeof(ModelData); // 1 million objects

    requiredLimits.limits.maxTextureDimension1D = 1440;
    requiredLimits.limits.maxTextureDimension2D = 5120;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    requiredLimits.limits.maxSampledTexturesPerShaderStage = 2;
    requiredLimits.limits.maxSamplersPerShaderStage = 1;

    DeviceDescriptor deviceDesc;
    deviceDesc.defaultQueue = QueueDescriptor{};
    deviceDesc.requiredFeaturesCount = 1;
    deviceDesc.requiredFeatures = (WGPUFeatureName*)&NativeFeature::VertexWritableStorage;
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
// todo
// bool Renderer::addModelToVertexBuffer() {

// }

// returns the vertex offset of this mesh in the vertex buffer.
int Renderer::addMeshToVertexBuffer(std::vector<Mesh::VertexData> vertexData) {
    std::cout << "adding mesh to vertex buffer" << std::endl;
    int vertexOffset = m_vertexCount;
    int offset = m_vertexCount * sizeof(VertexData);
    int newDataSize = (int)(vertexData.size() * sizeof(VertexData));
    m_queue.writeBuffer(m_vertexBuffer, offset, vertexData.data(), newDataSize);
    m_vertexCount = (int)(m_vertexCount + vertexData.size());
    return vertexOffset;
}

bool Renderer::initBuffers() {
    std::cout << "initializing buffers" << std::endl;

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = 10000000 * sizeof(VertexData); // 10,000,000 vertices
    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.label = "model matrix buffer";
    bufferDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    bufferDesc.size = 10 * sizeof(ModelData);
    m_modelBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.label = "uniform buffer";
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.size = sizeof(UniformData);
    m_uniformBuffer = m_device.createBuffer(bufferDesc);

    m_uniformData.time = 1.0;
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 100.0f);

    m_uniformData.model_matrix = mat4x4(1.0);

    m_queue.writeBuffer(m_uniformBuffer, 0, &m_uniformData, sizeof(UniformData));
    updateViewMatrix();

    return true;
}

bool Renderer::initTextures() {
    std::cout << "loading textures" << std::endl;
    m_albedoTexture = ResourceLoader::loadTexture(RESOURCE_DIR, "fourareen2K_albedo.jpg", m_device, m_albedoTextureView);
    m_normalTexture = ResourceLoader::loadTexture(RESOURCE_DIR, "fourareen2K_normals.png", m_device, m_normalTextureView);

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

    return true;
}

void Renderer::releaseTextures() {
    m_textureSampler.release();
    
    m_albedoTexture.destroy();
    m_albedoTexture.release();
    m_albedoTextureView.release();

    m_normalTexture.destroy();
    m_normalTexture.release();
    m_normalTextureView.release();
}

bool Renderer::initBindGroups() {
    std::cout << "initializing bind groups" << std::endl;
    std::vector<BindGroupLayoutEntry> bindGroupLayoutEntries(5, Default);
    // uniform layout
    bindGroupLayoutEntries[0].binding = 0;
    bindGroupLayoutEntries[0].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[0].buffer.type = BufferBindingType::Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof(UniformData);

    // albedo layout
    bindGroupLayoutEntries[1].binding = 1;
    bindGroupLayoutEntries[1].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[1].texture.sampleType = TextureSampleType::Float;
    bindGroupLayoutEntries[1].texture.multisampled = false;
    bindGroupLayoutEntries[1].texture.viewDimension = TextureViewDimension::_2D;

    // normal map layout
    bindGroupLayoutEntries[2].binding = 2;
    bindGroupLayoutEntries[2].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[2].texture.sampleType = TextureSampleType::Float;
    bindGroupLayoutEntries[2].texture.multisampled = false;
    bindGroupLayoutEntries[2].texture.viewDimension = TextureViewDimension::_2D;

    // texture sampler layout
    bindGroupLayoutEntries[3].binding = 3;
    bindGroupLayoutEntries[3].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[3].sampler.type = SamplerBindingType::Filtering;

    // model buffer layout
    bindGroupLayoutEntries[4].binding = 4;
    bindGroupLayoutEntries[4].visibility = ShaderStage::Vertex;
    bindGroupLayoutEntries[4].buffer.type = BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[4].buffer.minBindingSize = sizeof(ModelData);

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();

    m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    std::vector<BindGroupEntry> bindGroupEntries(5, Default);
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].offset = 0;
    bindGroupEntries[0].buffer = m_uniformBuffer;
    bindGroupEntries[0].size = sizeof(UniformData);

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].offset = 0;
    bindGroupEntries[1].textureView = m_albedoTextureView;
    
    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].offset = 0;
    bindGroupEntries[2].textureView = m_normalTextureView;

    bindGroupEntries[3].binding = 3;
    bindGroupEntries[3].offset = 0;
    bindGroupEntries[3].sampler = m_textureSampler;

    bindGroupEntries[4].binding = 4;
    bindGroupEntries[4].offset = 0;
    bindGroupEntries[4].buffer = m_modelBuffer;
    bindGroupEntries[4].size = 10 * sizeof(ModelData);

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

    std::vector<VertexAttribute> vertexAttributes(7);
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
    // modelId
    vertexAttributes[6].format = VertexFormat::Uint32;
    vertexAttributes[6].offset = offsetof(VertexData, modelID);
    vertexAttributes[6].shaderLocation = 6;

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

    renderPipelineDesc.primitive.cullMode = CullMode::None;
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

bool Renderer::initSwapChain() {
    std::cout << "initializing swap chain" << std::endl;
    SwapChainDescriptor swapChainDesc;
    swapChainDesc.format = m_preferredFormat;
    swapChainDesc.height = m_screenHeight;
    swapChainDesc.width = m_screenWidth;
    swapChainDesc.usage = TextureUsage::RenderAttachment;
    swapChainDesc.presentMode = PresentMode::Fifo;

    m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);

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
    
    return true;
}

void Renderer::updateProjectionMatrix() {
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 100.0f);
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
    releaseSwapChain();

    initDepthBuffer();
    initSwapChain();

    updateProjectionMatrix();
}