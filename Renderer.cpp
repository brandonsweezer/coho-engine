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

const float PI = 3.14159265358979323846f;

void Renderer::onFrame() {
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
    renderPassEncoder.draw(m_vertexCount, 1, 0, 0);

    renderPassEncoder.end();
    CommandBuffer commandBuffer = commandEncoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commandBuffer);
    
    m_swapChain.present();

    renderPassEncoder.release();
    commandBuffer.release();
    commandEncoder.release();
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
    requiredLimits.limits.minStorageBufferOffsetAlignment = 32;
    requiredLimits.limits.minUniformBufferOffsetAlignment = 64;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(UniformData);
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxVertexAttributes = 4;
    requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexData);
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexData);
    requiredLimits.limits.maxInterStageShaderComponents = 9;

    requiredLimits.limits.maxTextureDimension1D = 1440;
    requiredLimits.limits.maxTextureDimension2D = 5120;
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

    std::cout << "initializing queue" << std::endl;
    m_queue = m_device.getQueue();
    if (!m_queue) return false;

    adapter.release();

    return true;
}

bool Renderer::initBuffers() {
    std::cout << "initializing buffers" << std::endl;

    std::vector<VertexData> model;
    bool success = ResourceLoader::loadObj(RESOURCE_DIR "/teapot.obj", model);
    if (!success) {
        std::cerr << "failed to load obj file" << std::endl;
        return false;
    }

    m_vertexCount = (uint32_t)model.size();

    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;
    bufferDesc.size = model.size() * sizeof(VertexData);

    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    m_queue.writeBuffer(m_vertexBuffer, 0, model.data(), bufferDesc.size);

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

bool Renderer::initBindGroups() {
    std::cout << "initializing bind groups" << std::endl;
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