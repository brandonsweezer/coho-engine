#include "ComputeModule.h"
#include "../ResourceLoader.h"
#include "../noise/RandomNumberGenerator.h"
#include "../utilities/io/ExportTexture.h"
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
using namespace wgpu;

ComputeModule::ComputeModule() {
    if (!init()) {
        std::cout << "failed to initialize compute module!" << std::endl;
    }
    std::vector<float> P = { 
        151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
        140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
        247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
        57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
        74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
        60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
        65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
        200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
        52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
        207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
        119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
        129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
        218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
        81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
        184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
        222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180
    };

    std::vector<glm::vec4> P2(256);
    for (int i = 0; i < P.size(); ++i) {
        P2[i] = glm::vec4(P[i], 0, 0, 0);
    }

    std::vector<glm::vec4> G = {
        glm::vec4(1.0,  1.0, 0.0, 1.0), glm::vec4(-1.0, 1.0,  0.0, 1.0), glm::vec4(1.0, -1.0, 0.0, 1.0), glm::vec4( -1.0, -1.0, 0.0, 1.0),
        glm::vec4(1.0,  0.0, 1.0, 1.0), glm::vec4(-1.0, 0.0,  1.0, 1.0), glm::vec4(1.0,  0.0,  -1.0, 1.0), glm::vec4(-1.0, 0.0, -1.0, 1.0),
        glm::vec4(0.0,  1.0,  1.0, 1.0),  glm::vec4(0.0, -1.0, 1.0, 1.0), glm::vec4(0.0,  1.0, -1.0, 1.0), glm::vec4(0.0, -1.0, -1.0, 1.0),
        glm::vec4(1.0,  1.0,  0.0, 1.0), glm::vec4(0.0,  -1.0, 1.0, 1.0),  glm::vec4(-1.0, 1.0, 0.0, 1.0),  glm::vec4(0.0, -1.0, -1.0, 1.0)
    };
    Queue queue = m_device.getQueue();
    ImageCopyTexture destination;
    destination.texture = m_permutationTexture;
    destination.mipLevel = 0;
    destination.origin = {0,0,0};
    destination.aspect = TextureAspect::All;
    TextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = sizeof(uint32_t) * 256;
    source.rowsPerImage = 1;

    queue.writeTexture(destination, P2.data(), 256 * 4 * sizeof(uint32_t), source, {256, 1, 1});
    destination.texture = m_gradientTexture;
    source.bytesPerRow = sizeof(float) * 4 * 16;
    queue.writeTexture(destination, G.data(), 16 * 4 * sizeof(float), source, {16, 1, 1});

    queue.release();

    std::cout << "computing noise texture" << std::endl;
    onCompute();
}

ComputeModule::~ComputeModule() {
    terminateComputePipeline();
    terminateShaderModule();
    terminateBindGroups();
    terminateTextures();
    terminateBuffers();
    terminateDevice();
}

bool ComputeModule::init() {
    if (!initDevice()) {
        std::cout << "failed to init device" << std::endl;
        return false;
    }
    if (!initBuffers()) {
        std::cout << "failed to init buffers" << std::endl;
        return false;
    }
    if (!initTextures()) {
        std::cout << "failed to init textures" << std::endl;
        return false;
    }
    if (!initBindGroups()) {
        std::cout << "failed to init bind groups" << std::endl;
        return false;
    }
    if (!initShaderModule()) {
        std::cout << "failed to init shader module" << std::endl;
        return false;
    }
    if (!initComputePipeline()) {
        std::cout << "failed to init compute pipeline" << std::endl;
        return false;
    }
    return true;
}

void ComputeModule::onCompute() {
    Queue queue = m_device.getQueue();
    CommandEncoder encoder = m_device.createCommandEncoder(CommandEncoderDescriptor{});
    ComputePassDescriptor computeDesc;
    ComputePassEncoder computePass = encoder.beginComputePass(computeDesc);
    
    // we want a 1024 x 1024 noise texture.
    computePass.setBindGroup(0, m_bindGroup, 0, 0);
    computePass.setPipeline(m_computePipeline);
    
    m_uniformData.frequency = 0.001;
    m_uniformData.scale = 1.0;
    m_uniformData.seed = 453253;
    m_uniformData.octaves = 1;
    queue.writeBuffer(m_uniformBuffer, 0, &m_uniformData, sizeof(UniformData));

    computePass.dispatchWorkgroups(32, 32, 1);
    computePass.end();
    encoder.copyBufferToBuffer(m_outputBuffer, 0, m_mapBuffer, 0, 1024 * 1024 * 1 * sizeof(float));

    CommandBuffer commands = encoder.finish(CommandBufferDescriptor{});
    queue.submit(commands);

    encoder.release();
    commands.release();

    // Print output
    bool done = false;
    auto handle = m_mapBuffer.mapAsync(MapMode::Read, 0, 1024 * 1024 * 1 * sizeof(float), [&](BufferMapAsyncStatus status) {
        if (status == BufferMapAsyncStatus::Success) {
            const float* output = (const float*)m_mapBuffer.getConstMappedRange(0, 1024 * 1024 * 1 * sizeof(float));
            std::cout << output[1024*1024 - 1] << std::endl;
            ResourceLoader::ImageData* img = ResourceLoader::loadImage(RESOURCE_DIR, "brick_diffuse.jpg");
            ExportTexture::exportPng(RESOURCE_DIR, "brick_diffuse_clone.png", img->width, img->height, img->channels, img->data);
            ExportTexture::exportPng(RESOURCE_DIR, "noise.png", 512, 512, 1, output);
            m_mapBuffer.unmap();
        }
        done = true;
    });

    while (!done) {
        queue.submit(0, nullptr);
    }
    queue.release();
}

bool ComputeModule::initBuffers() {
    std::cout << "initializing input buffer" << std::endl;
    BufferDescriptor bufferDesc;
    bufferDesc.label = "input buffer";
    bufferDesc.mappedAtCreation = false;
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage;
    bufferDesc.size = 1024 * 1024 * 1 * sizeof(float);
    m_inputBuffer = m_device.createBuffer(bufferDesc);

    std::cout << "initializing output buffer" << std::endl;
    bufferDesc.label = "input buffer";
    bufferDesc.mappedAtCreation = false;
    bufferDesc.usage = BufferUsage::CopySrc | BufferUsage::Storage;
    bufferDesc.size = 1024 * 1024 * 1 * sizeof(float);
    m_outputBuffer = m_device.createBuffer(bufferDesc);

    std::cout << "initializing map buffer" << std::endl;
    bufferDesc.label = "map buffer";
    bufferDesc.mappedAtCreation = false;
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::MapRead;
    bufferDesc.size = 1024 * 1024 * 1 * sizeof(float);
    m_mapBuffer = m_device.createBuffer(bufferDesc);

    std::cout << "initializing uniform buffer" << std::endl;
    bufferDesc.label = "uniform buffer";
    bufferDesc.mappedAtCreation = false;
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = sizeof(UniformData);
    m_uniformBuffer = m_device.createBuffer(bufferDesc);

    return true;
}

bool ComputeModule::initTextures() {
    TextureDescriptor texDesc;
    // permutation texture
    texDesc.dimension = TextureDimension::_1D;
    texDesc.format = TextureFormat::R32Uint;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;
    texDesc.size.width = 256 * sizeof(uint32_t);
    texDesc.size.height = 1;
    texDesc.size.depthOrArrayLayers = 1;
    texDesc.usage = TextureUsage::CopyDst | TextureUsage::TextureBinding;
    texDesc.viewFormatCount = 1;
    texDesc.viewFormats = &TextureFormat::R32Uint;
    m_permutationTexture = m_device.createTexture(texDesc);

    TextureViewDescriptor texViewDesc;
    texViewDesc.aspect = TextureAspect::All;
    texViewDesc.arrayLayerCount = 1;
    texViewDesc.baseArrayLayer = 0;
    texViewDesc.mipLevelCount = 1;
    texViewDesc.baseMipLevel = 0;
    texViewDesc.dimension = TextureViewDimension::_1D;
    texViewDesc.format = TextureFormat::R32Uint;
    m_permutationTextureView = m_permutationTexture.createView(texViewDesc);

    // gradient texture - maybe enumerate everything so this code is more portable
    texDesc.dimension = TextureDimension::_1D;
    texDesc.format = TextureFormat::RGBA32Float;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;
    texDesc.size.width = 16 * 4 * sizeof(float);
    texDesc.size.height = 1;
    texDesc.size.depthOrArrayLayers = 1;
    texDesc.usage = TextureUsage::CopyDst | TextureUsage::TextureBinding;
    texDesc.viewFormatCount = 1;
    texDesc.viewFormats = &TextureFormat::RGBA32Float;
    m_gradientTexture = m_device.createTexture(texDesc);

    texViewDesc.aspect = TextureAspect::All;
    texViewDesc.arrayLayerCount = 1;
    texViewDesc.baseArrayLayer = 0;
    texViewDesc.mipLevelCount = 1;
    texViewDesc.baseMipLevel = 0;
    texViewDesc.dimension = TextureViewDimension::_1D;
    texViewDesc.format = TextureFormat::RGBA32Float;
    m_gradientTextureView = m_gradientTexture.createView(texViewDesc);

    // permutation sampler
    SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = AddressMode::Repeat;
    samplerDesc.addressModeV = AddressMode::Repeat;
    samplerDesc.addressModeW = AddressMode::Repeat;
    samplerDesc.maxAnisotropy = 1;
    m_permutationSampler = m_device.createSampler(samplerDesc);

    // gradient sampler
    m_gradientSampler = m_device.createSampler(samplerDesc);

    return true;
}

void ComputeModule::terminateTextures() {
    m_gradientTexture.destroy();
    m_gradientTexture.release();
    m_gradientSampler.release();

    m_permutationTexture.destroy();
    m_permutationTexture.release();
    m_permutationSampler.release();
}

void ComputeModule::terminateBuffers() {
    m_inputBuffer.destroy();
    m_inputBuffer.release();

    m_outputBuffer.destroy();
    m_outputBuffer.release();

    m_mapBuffer.destroy();
    m_mapBuffer.release();
}

bool ComputeModule::initBindGroups() {
    std::vector<BindGroupLayoutEntry> bindGroupLayoutEntries(7);
    bindGroupLayoutEntries[0].binding = 0;
    bindGroupLayoutEntries[0].buffer.type = BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[0].visibility = ShaderStage::Compute;
    bindGroupLayoutEntries[0].buffer.hasDynamicOffset = false;

    bindGroupLayoutEntries[1].binding = 1;
    bindGroupLayoutEntries[1].buffer.type = BufferBindingType::Storage;
    bindGroupLayoutEntries[1].visibility = ShaderStage::Compute;
    bindGroupLayoutEntries[1].buffer.hasDynamicOffset = false;

    bindGroupLayoutEntries[2].binding = 2;
    bindGroupLayoutEntries[2].buffer.type = BufferBindingType::Uniform;
    bindGroupLayoutEntries[2].visibility = ShaderStage::Compute;
    bindGroupLayoutEntries[2].buffer.hasDynamicOffset = false;

    // permutation texture
    bindGroupLayoutEntries[3].binding = 3;
    bindGroupLayoutEntries[3].texture.multisampled = false;
    bindGroupLayoutEntries[3].texture.sampleType = TextureSampleType::Uint;
    bindGroupLayoutEntries[3].texture.viewDimension = TextureViewDimension::_1D;
    bindGroupLayoutEntries[3].texture.nextInChain = nullptr;
    bindGroupLayoutEntries[3].visibility = ShaderStage::Compute;

    // permutation sampler
    bindGroupLayoutEntries[4].binding = 4;
    bindGroupLayoutEntries[4].sampler.nextInChain = nullptr;
    bindGroupLayoutEntries[4].sampler.type = SamplerBindingType::NonFiltering;
    bindGroupLayoutEntries[4].visibility = ShaderStage::Compute;

    // gradient texture
    bindGroupLayoutEntries[5].binding = 5;
    bindGroupLayoutEntries[5].texture.multisampled = false;
    bindGroupLayoutEntries[5].texture.sampleType = TextureSampleType::UnfilterableFloat;
    bindGroupLayoutEntries[5].texture.viewDimension = TextureViewDimension::_1D;
    bindGroupLayoutEntries[3].texture.nextInChain = nullptr;
    bindGroupLayoutEntries[5].visibility = ShaderStage::Compute;

    // gradient sampler
    bindGroupLayoutEntries[6].binding = 6;
    bindGroupLayoutEntries[6].sampler.nextInChain = nullptr;
    bindGroupLayoutEntries[6].sampler.type = SamplerBindingType::NonFiltering;
    bindGroupLayoutEntries[6].visibility = ShaderStage::Compute;

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.entryCount = bindGroupLayoutEntries.size();

    std::cout << "creating bind group layout" << std::endl;
    m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    std::vector<BindGroupEntry> bindGroupEntries(7);
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].buffer = m_inputBuffer;
    bindGroupEntries[0].offset = 0;
    bindGroupEntries[0].size = 1024 * 1024 * 1 * sizeof(float);

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].buffer = m_outputBuffer;
    bindGroupEntries[1].offset = 0;
    bindGroupEntries[1].size = 1024 * 1024 * 1 * sizeof(float);

    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].buffer = m_uniformBuffer;
    bindGroupEntries[2].offset = 0;
    bindGroupEntries[2].size = sizeof(UniformData);

    // permutation texture
    bindGroupEntries[3].binding = 3;
    bindGroupEntries[3].textureView = m_permutationTextureView;
    
    // perm sampler
    bindGroupEntries[4].binding = 4;
    bindGroupEntries[4].sampler = m_permutationSampler;

    // gradient texture
    bindGroupEntries[5].binding = 5;
    bindGroupEntries[5].textureView = m_gradientTextureView;

    // gradient sampler
    bindGroupEntries[6].binding = 6;
    bindGroupEntries[6].sampler = m_gradientSampler;

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.entries = bindGroupEntries.data();
    bindGroupDesc.entryCount = bindGroupEntries.size();
    bindGroupDesc.layout = m_bindGroupLayout;

    std::cout << "creating bind group" << std::endl;
    m_bindGroup = m_device.createBindGroup(bindGroupDesc);

    return true;
}

void ComputeModule::terminateBindGroups() {
    m_bindGroup.release();
    m_bindGroupLayout.release();
}

bool ComputeModule::initShaderModule() {
    std::cout << "init shader module" << std::endl;
    std::string shaderCode = ResourceLoader::loadShaderCode(RESOURCE_DIR "/noise.wgsl");
    ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = shaderCode.c_str();
    wgslDesc.chain.next = nullptr;
    wgslDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    ShaderModuleDescriptor shaderDesc;
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    shaderDesc.nextInChain = &wgslDesc.chain;
    shaderDesc.label = "compute shader";

    m_shaderModule = m_device.createShaderModule(shaderDesc);

    return true;
}

void ComputeModule::terminateShaderModule() {
    m_shaderModule.release();
}

bool ComputeModule::initComputePipeline() {
    std::cout << "init pipeline layout" << std::endl;
    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutDesc);

    std::cout << "init compute pipeline" << std::endl;
    ComputePipelineDescriptor pipelineDesc = Default;
    pipelineDesc.compute.entryPoint = "main";
    pipelineDesc.compute.module = m_shaderModule;
    pipelineDesc.layout = m_pipelineLayout;
    pipelineDesc.label = "compute pipeline";
        std::cout << "sending request" << std::endl;
    m_computePipeline = m_device.createComputePipeline(pipelineDesc);
        std::cout << "created the pipeline" << std::endl;


    return true;
}

void ComputeModule::terminateComputePipeline() {
    m_pipelineLayout.release();
    m_computePipeline.release();
}
bool ComputeModule::initDevice() {
    m_instance = createInstance(InstanceDescriptor{});

    std::cout << "initializing adapter" << std::endl;
    RequestAdapterOptions adapterOptions;
    adapterOptions.forceFallbackAdapter = false;
    adapterOptions.powerPreference = PowerPreference::Undefined;
    Adapter adapter = m_instance.requestAdapter(adapterOptions);
    if (!adapter) return false;

    std::cout << "initializing device" << std::endl;
    RequiredLimits requiredLimits;
    requiredLimits.limits.minStorageBufferOffsetAlignment = 64;
    requiredLimits.limits.minUniformBufferOffsetAlignment = 64;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(UniformData);
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxBindingsPerBindGroup = 7;
    requiredLimits.limits.maxTextureDimension1D = 1024;
    requiredLimits.limits.maxTextureDimension2D = 1;
    requiredLimits.limits.maxTextureDimension3D = 1;
    requiredLimits.limits.maxSampledTexturesPerShaderStage = 2;
    requiredLimits.limits.maxSamplersPerShaderStage = 2;
    requiredLimits.limits.maxComputeWorkgroupSizeX = 32;
    requiredLimits.limits.maxComputeWorkgroupSizeY = 32;
    requiredLimits.limits.maxComputeWorkgroupSizeZ = 1;
    requiredLimits.limits.maxComputeInvocationsPerWorkgroup = 1024;
    requiredLimits.limits.maxComputeWorkgroupsPerDimension = 32;
    requiredLimits.limits.maxBufferSize = 1024*1024*4 * sizeof(float);
    requiredLimits.limits.maxInterStageShaderComponents = 0;
    requiredLimits.limits.maxStorageBuffersPerShaderStage = 2;
    requiredLimits.limits.maxStorageBufferBindingSize = 1024*1024*4 * sizeof(float);

    DeviceDescriptor deviceDesc;
    deviceDesc.defaultQueue = QueueDescriptor{};
    deviceDesc.requiredFeatureCount = 0;
    // special conversion for native features
    std::vector<WGPUFeatureName> reqFeatures{
        // (WGPUFeatureName)NativeFeature::TextureBindingArray,
        // (WGPUFeatureName)NativeFeature::SampledTextureAndStorageBufferArrayNonUniformIndexing
    };
    deviceDesc.requiredFeatures = reqFeatures.data();
    deviceDesc.requiredLimits = &requiredLimits;
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const * message, void * userdata) {
        std::cout << "Device lost! Reason: " << reason << userdata << "(\n" << message << "\n)" << std::endl;
    };

    std::cout << "requesting device" << std::endl;
    m_device = adapter.requestDevice(deviceDesc);
    std::cout << "1" << std::endl;
    if (!m_device) return false;
    std::cout << "device exists!" << std::endl;
    m_deviceErrorCallback = m_device.setUncapturedErrorCallback([](ErrorType type, char const * message) {
        std::cout << "Device error! " << type << "(\n" << message << "\n)" << std::endl;
    });

    adapter.release();

    return true;
}

void ComputeModule::terminateDevice() {
    m_device.destroy();
    m_device.release();

    m_deviceErrorCallback.release();

    m_instance.release();
}