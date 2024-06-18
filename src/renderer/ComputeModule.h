#pragma once
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>

using namespace wgpu;

class ComputeModule {
public:
    ComputeModule();
    ~ComputeModule();

    void onCompute();
private:
    bool init();

    bool initShaderModule();
    void terminateShaderModule();

    bool initComputePipeline();
    void terminateComputePipeline();

    bool initBindGroups();
    void terminateBindGroups();

    bool initBuffers();
    void terminateBuffers();

    bool initTextures();
    void terminateTextures();

    bool initDevice();
    void terminateDevice();
    
private:
    struct UniformData {
        float frequency;
        float scale;
        uint32_t seed;
        uint32_t octaves;
        // float padding[0];
    };

    ShaderModule m_shaderModule = nullptr;
    Buffer m_dataBuffer = nullptr;

    Instance m_instance = nullptr;
    Device m_device = nullptr;
    std::unique_ptr<wgpu::ErrorCallback> m_deviceErrorCallback;

    PipelineLayout m_pipelineLayout = nullptr;
    ComputePipeline m_computePipeline = nullptr;
    BindGroup m_bindGroup = nullptr;
    BindGroupLayout m_bindGroupLayout = nullptr;

    Buffer m_inputBuffer = nullptr;
    Buffer m_outputBuffer = nullptr;
    Buffer m_mapBuffer = nullptr;
    Buffer m_uniformBuffer = nullptr;

    // perlin noise specific resources
    wgpu::Texture m_permutationTexture = nullptr; // 1d texture with permutation values
    Sampler m_permutationSampler = nullptr;
    TextureView m_permutationTextureView = nullptr;
    
    wgpu::Texture m_gradientTexture = nullptr; // 1d texture with gradient values?
    Sampler m_gradientSampler = nullptr;
    TextureView m_gradientTextureView = nullptr;

    UniformData m_uniformData;
};