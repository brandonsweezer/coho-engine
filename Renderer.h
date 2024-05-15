#pragma once
#include <SDL2/SDL.h>
#include <webgpu/webgpu.hpp>
#include <sdl2webgpu/sdl2webgpu.h>
#include <glm/glm.hpp>

class Renderer
{

public:
    bool init();
    bool isRunning();
    void onFrame();
    void terminate();

private:
    bool initInstance();

    bool initWindowAndSurface();

    bool initAdapter();

    bool initDevice();

    bool initQueue();

    bool initRenderPipeline();

    bool initShaderModule();

    bool initBuffers();

    bool initBindGroup();

    bool initSwapChain();

    bool initDepthBuffer();

private:
    int m_screenWidth = 640;
    int m_screenHeight = 480;

    bool m_isRunning = true;


    struct UniformData {
        glm::mat4x4 model_matrix;
        glm::mat4x4 view_matrix;
        glm::mat4x4 projection_matrix;
        float time;
        float padding[15]; // need to chunk into 4x4x4 sections (4x4 floats)
    };
    UniformData m_uniformData;

    wgpu::Instance m_instance = nullptr;
    SDL_Window* m_window = nullptr;
    wgpu::Surface m_surface = nullptr;
    wgpu::SwapChain m_swapChain = nullptr;
    wgpu::Device m_device = nullptr;
    wgpu::Queue m_queue = nullptr;
    wgpu::ShaderModule m_shaderModule = nullptr;

    wgpu::TextureFormat m_preferredFormat = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;

    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
    wgpu::BindGroup m_bindGroup = nullptr;
    wgpu::RenderPipeline m_renderPipeline = nullptr;

    std::unique_ptr<wgpu::ErrorCallback> m_deviceErrorCallback;

    wgpu::Buffer m_vertexBuffer = nullptr;
    int m_vertexCount = 0;

    wgpu::Buffer m_uniformBuffer = nullptr;

    wgpu::Texture m_depthTexture = nullptr;
    wgpu::TextureView m_depthTextureView = nullptr;

};