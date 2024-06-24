#pragma once
#include "../../memory/Pipeline.h"
#include "../../memory/Buffer.h"
#include "../../memory/Shader.h"
#include "../../ecs/components/Mesh.h"

namespace coho {
class DefaultPipeline: Pipeline {
public:
DefaultPipeline(
        std::shared_ptr<Buffer> vertexBuffer,
        std::shared_ptr<Buffer> indexBuffer,
        std::shared_ptr<Buffer> uniformBuffer,
        std::shared_ptr<Buffer> modelBuffer,
        std::shared_ptr<Buffer> materialBuffer,
        std::shared_ptr<Shader> vertexShader,
        std::shared_ptr<Shader> fragmentShader
        ) {
    m_vertexBuffer = vertexBuffer;
    m_indexBuffer = indexBuffer;
    m_uniformBuffer = uniformBuffer;
    m_modelBuffer = modelBuffer;
    m_materialBuffer = materialBuffer;

    m_vertexShader = vertexShader;
    m_fragmentShader = fragmentShader;
}

~DefaultPipeline() {
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_uniformBuffer.reset();
    m_modelBuffer.reset();
    m_materialBuffer.reset();

    m_textureSampler.release();
    m_bindGroupLayout.release();
    m_bindGroup.release();
}

bool DefaultPipeline::init(
        wgpu::Device device,
        wgpu::TextureFormat preferredFormat,
        std::vector<wgpu::TextureView> textureViewArray
    ) {

    if (!initSampler(device)) {
        std::cout << "failed to init sampler" << std::endl;
        return false;
    }

    if (!initBindings(device, textureViewArray)) {
        std::cout << "failed to init bindings" << std::endl;
        return false;
    }

    wgpu::RenderPipelineDescriptor renderPipelineDesc;

    std::vector<wgpu::VertexAttribute> vertexAttributes(6);
    // position
    vertexAttributes[0].format = wgpu::VertexFormat::Float32x3;
    vertexAttributes[0].offset = offsetof(Mesh::VertexData, position);
    vertexAttributes[0].shaderLocation = 0;
    // normal
    vertexAttributes[1].format = wgpu::VertexFormat::Float32x3;
    vertexAttributes[1].offset = offsetof(Mesh::VertexData, normal);
    vertexAttributes[1].shaderLocation = 1;
    // color
    vertexAttributes[2].format = wgpu::VertexFormat::Float32x3;
    vertexAttributes[2].offset = offsetof(Mesh::VertexData, color);
    vertexAttributes[2].shaderLocation = 2;
    // tangent
    vertexAttributes[3].format = wgpu::VertexFormat::Float32x3;
    vertexAttributes[3].offset = offsetof(Mesh::VertexData, tangent);
    vertexAttributes[3].shaderLocation = 3;
    // bitangent 
    vertexAttributes[4].format = wgpu::VertexFormat::Float32x3;
    vertexAttributes[4].offset = offsetof(Mesh::VertexData, bitangent);
    vertexAttributes[4].shaderLocation = 4;
    // uv
    vertexAttributes[5].format = wgpu::VertexFormat::Float32x2;
    vertexAttributes[5].offset = offsetof(Mesh::VertexData, uv);
    vertexAttributes[5].shaderLocation = 5;

    wgpu::VertexBufferLayout vertexBufferLayout;
    vertexBufferLayout.arrayStride = sizeof(Mesh::VertexData);
    vertexBufferLayout.attributes = vertexAttributes.data();
    vertexBufferLayout.attributeCount = (uint32_t)vertexAttributes.size();
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    renderPipelineDesc.vertex.bufferCount = 1;
    renderPipelineDesc.vertex.buffers = &vertexBufferLayout;
    renderPipelineDesc.vertex.constantCount = 0;
    renderPipelineDesc.vertex.constants = 0;
    renderPipelineDesc.vertex.entryPoint = "vs_main";
    
    renderPipelineDesc.vertex.module = m_vertexShader->getShaderModule();

    wgpu::BlendState blendState;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;

    blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blendState.alpha.dstFactor = wgpu::BlendFactor::One;
    blendState.alpha.operation = wgpu::BlendOperation::Add;

    wgpu::ColorTargetState colorTargetState;
    colorTargetState.format = preferredFormat;
    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState;
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.entryPoint = "fs_main";
    fragmentState.module = m_fragmentShader->getShaderModule();
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;
    renderPipelineDesc.fragment = &fragmentState;

    renderPipelineDesc.primitive.cullMode = wgpu::CullMode::Back;
    renderPipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList; 
    renderPipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    wgpu::DepthStencilState depthStencilState = wgpu::Default;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	depthStencilState.format = m_depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

    renderPipelineDesc.depthStencil = &depthStencilState;

    renderPipelineDesc.multisample.count = 1;
    renderPipelineDesc.multisample.mask = ~0u;
	renderPipelineDesc.multisample.alphaToCoverageEnabled = false;
    
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
    renderPipelineDesc.layout = device.createPipelineLayout(pipelineLayoutDesc);
    
    m_renderPipeline = device.createRenderPipeline(renderPipelineDesc);
    if (m_renderPipeline == nullptr) {
        std::cout << "render pipeline wasn't created" << std::endl;
        return false;
    }
    return true;
}

wgpu::RenderPipeline DefaultPipeline::getRenderPipeline() {
    return m_renderPipeline;
}

private:

bool DefaultPipeline::initBindings(wgpu::Device device, std::vector<wgpu::TextureView> textureViewArray) {
    std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries(5, wgpu::Default);
    // uniform layout
    bindGroupLayoutEntries[0].binding = 0;
    bindGroupLayoutEntries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof(UniformData);

    // array of textures
    wgpu::BindGroupLayoutEntryExtras textureArrayLayout;
    textureArrayLayout.count = (uint32_t)textureViewArray.size();
    textureArrayLayout.chain.sType = (WGPUSType)WGPUSType_BindGroupLayoutEntryExtras;
    textureArrayLayout.chain.next = nullptr;
    bindGroupLayoutEntries[1].binding = 1;
    bindGroupLayoutEntries[1].visibility = wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntries[1].texture.multisampled = false;
    bindGroupLayoutEntries[1].texture.viewDimension = wgpu::TextureViewDimension::_2D;
    bindGroupLayoutEntries[1].texture.sampleType = wgpu::TextureSampleType::Float;
    bindGroupLayoutEntries[1].nextInChain = &textureArrayLayout.chain;

    // texture sampler layout
    bindGroupLayoutEntries[2].binding = 2;
    bindGroupLayoutEntries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntries[2].sampler.type = wgpu::SamplerBindingType::Filtering;

    // model buffer layout
    bindGroupLayoutEntries[3].binding = 3;
    bindGroupLayoutEntries[3].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[3].buffer.minBindingSize = sizeof(ModelData);

    // material buffer layout
    bindGroupLayoutEntries[4].binding = 4;
    bindGroupLayoutEntries[4].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bindGroupLayoutEntries[4].buffer.minBindingSize = sizeof(MaterialData);

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();

    m_bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

    std::vector<wgpu::BindGroupEntry> bindGroupEntries(5, wgpu::Default);
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].offset = 0;
    bindGroupEntries[0].buffer = m_uniformBuffer->getBuffer();
    bindGroupEntries[0].size = sizeof(UniformData);

    wgpu::BindGroupEntryExtras textureArray;
    textureArray.textureViewCount = textureViewArray.size();
    textureArray.textureViews = (WGPUTextureView*)textureViewArray.data();
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
    bindGroupEntries[3].buffer = m_modelBuffer->getBuffer();
    bindGroupEntries[3].size = m_modelBuffer->getSize();

    bindGroupEntries[4].binding = 4;
    bindGroupEntries[4].offset = 0;
    bindGroupEntries[4].buffer = m_materialBuffer->getBuffer();
    bindGroupEntries[4].size = m_materialBuffer->getSize();

    wgpu::BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.entries = bindGroupEntries.data();
    bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
    bindGroupDesc.layout = m_bindGroupLayout;

    m_bindGroup = device.createBindGroup(bindGroupDesc);

    return true;
}

bool DefaultPipeline::initSampler(wgpu::Device device) {
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
    samplerDesc.addressModeV = wgpu::AddressMode::Repeat;
    samplerDesc.addressModeW = wgpu::AddressMode::Repeat;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    samplerDesc.lodMaxClamp = 8.0;
    samplerDesc.lodMinClamp = 0.0;
    samplerDesc.maxAnisotropy = 1;
    samplerDesc.compare = wgpu::CompareFunction::Undefined;
    m_textureSampler = device.createSampler(samplerDesc);

    return true;
}

public:
struct UniformData {
    glm::mat4x4 view_matrix;
    glm::mat4x4 projection_matrix;
    glm::vec3 camera_world_position;
    float time;
    float padding[3]; // need to chunk into 4x4x4 sections (4x4 floats)
};
struct ModelData {
    glm::mat4x4 transform;
    uint32_t materialIndex;
    uint32_t isSkybox;
    float padding[2]; // need to chunk into 4x4x4 sections (4x4 floats)
};

struct MaterialData {
    glm::vec3 baseColor;
    uint32_t diffuseTextureIndex;
    uint32_t normalTextureIndex;
    float roughness;
    float padding[2];
};
// todo: make bind group it's own thing
wgpu::BindGroup m_bindGroup = nullptr;

private:
    wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;
    std::shared_ptr<Buffer> m_uniformBuffer;
    std::shared_ptr<Buffer> m_modelBuffer;
    std::shared_ptr<Buffer> m_materialBuffer;

    wgpu::Sampler m_textureSampler = nullptr;

    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;

};
}