#include "Renderer.h"
#include "ResourceLoader.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <webgpu/webgpu.hpp>
#include <sdl2webgpu/sdl2webgpu.h>
using namespace wgpu;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using mat4x4 = glm::mat4x4;
using VertexData = ResourceLoader::VertexData;

void Renderer::onFrame() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                m_isRunning = false;
                break;
            default:
                break;
        }
    }
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
    renderPassEncoder.draw(m_vertexCount, 1, 0, 0);

    renderPassEncoder.end();
    CommandBuffer commandBuffer = commandEncoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commandBuffer);
    
    m_swapChain.present();

    renderPassEncoder.release();
    commandBuffer.release();
    commandEncoder.release();
    currentTextureView.release();
}

bool Renderer::init() {
    if (!initInstance()) return false;
    if (!initWindowAndSurface()) return false;
    if (!initDevice()) return false;
    if (!initQueue()) return false;
    if (!initShaderModule()) return false;
    if (!initBuffers()) return false;
    if (!initBindGroup()) return false;
    if (!initRenderPipeline()) return false;
    if (!initSwapChain()) return false;
    if (!initDepthBuffer()) return false;

    return true;
}

bool Renderer::isRunning() {
    return m_isRunning;
}

void Renderer::terminate() {

    m_renderPipeline.release();
    m_bindGroupLayout.release();
    m_bindGroup.release();
    
    m_vertexBuffer.destroy();
    m_vertexBuffer.release();
    m_uniformBuffer.destroy();
    m_uniformBuffer.release();

    m_shaderModule.release();
    m_queue.release();
    m_device.release();
    m_surface.release();
    SDL_RELEASE(m_window);
    m_instance.release();
    SDL_Quit();
}

bool Renderer::initInstance() {
    m_instance = createInstance(InstanceDescriptor{});
    if (!m_instance) return false;
    return true;
}

bool Renderer::initWindowAndSurface() {
    std::cout << "initializing SDL" << std::endl;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return false;
    }


    std::cout << "initializing window" << std::endl;
    m_window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screenWidth, m_screenHeight, 0);
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
    requiredLimits.limits.minStorageBufferOffsetAlignment = 32;
    requiredLimits.limits.minUniformBufferOffsetAlignment = 64;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(UniformData);
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxVertexAttributes = 4;
    requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexData);
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexData);
    requiredLimits.limits.maxInterStageShaderComponents = 6;

    requiredLimits.limits.maxTextureDimension1D = 1920;
    requiredLimits.limits.maxTextureDimension2D = 1920;
    requiredLimits.limits.maxTextureArrayLayers = 1;

    DeviceDescriptor deviceDesc;
    deviceDesc.defaultQueue = QueueDescriptor{};
    deviceDesc.requiredFeaturesCount = 0;
    deviceDesc.requiredFeatures = nullptr;
    deviceDesc.requiredLimits = &requiredLimits;
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const * message, void * userdata) {
        std::cout << "Device lost! Reason: " << reason << userdata << "(\n" << message << "\n)" << std::endl;
    };

    m_device = adapter.requestDevice(deviceDesc);
    if (!m_device) return false;
    m_deviceErrorCallback = m_device.setUncapturedErrorCallback([](ErrorType type, char const * message) {
        std::cout << "Device error! " << type << "(\n" << message << "\n)" << std::endl;
    });

    adapter.release();

    return true;
}

bool Renderer::initQueue() {
    std::cout << "initializing queue" << std::endl;
    m_queue = m_device.getQueue();
    if (!m_queue) return false;

    return true;
}

bool Renderer::initBuffers() {
    std::cout << "initializing buffers" << std::endl;

    std::vector<VertexData> teapot;
    bool success = ResourceLoader::loadObj(RESOURCE_DIR "/teapot.obj", teapot);
    if (!success) {
        std::cerr << "failed to load obj file" << std::endl;
        return false;
    }

    m_vertexCount = (uint32_t)teapot.size();

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = teapot.size() * sizeof(VertexData);

    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    m_queue.writeBuffer(m_vertexBuffer, 0, teapot.data(), bufferDesc.size);

    bufferDesc.label = "uniform buffer";
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.size = sizeof(UniformData);
    m_uniformBuffer = m_device.createBuffer(bufferDesc);

    m_uniformData.time = 1.0;
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    m_uniformData.projection_matrix = glm::perspective(45.0f * 3.14f / 180.0f, aspectRatio, 0.001f, 100.0f);

    m_uniformData.model_matrix = mat4x4(1.0);
    m_uniformData.view_matrix = glm::lookAt(vec3(50.0f, 10.0f, 40.0f), vec3(0.0f), vec3(0, 1.0f, 0));

    m_queue.writeBuffer(m_uniformBuffer, 0, &m_uniformData, sizeof(UniformData));

    return true;
}

bool Renderer::initBindGroup() {
    std::cout << "initializing bind group" << std::endl;
    std::vector<BindGroupLayoutEntry> bindGroupLayoutEntries(1, Default);
    bindGroupLayoutEntries[0].binding = 0;
    bindGroupLayoutEntries[0].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    bindGroupLayoutEntries[0].buffer.type = BufferBindingType::Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof(UniformData);

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();

    m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    std::vector<BindGroupEntry> bindGroupEntries(1, Default);
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].offset = 0;
    bindGroupEntries[0].buffer = m_uniformBuffer;
    bindGroupEntries[0].size = sizeof(UniformData);

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

    std::vector<VertexAttribute> vertexAttributes(4);
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
    // uv
    vertexAttributes[3].format = VertexFormat::Float32x2;
    vertexAttributes[3].offset = offsetof(VertexData, uv);
    vertexAttributes[3].shaderLocation = 3;

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

    m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
    
    return true;
}