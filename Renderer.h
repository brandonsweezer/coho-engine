#pragma once
#include <SDL2/SDL.h>
#include <webgpu/webgpu.hpp>
#include <sdl2webgpu/sdl2webgpu.h>
#include <glm/glm.hpp>
#include <vector>

class Renderer
{

public:
    bool init();
    bool isRunning();
    void onFrame();
    void terminate();

private:
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

    bool initBindGroups();
    void releaseBindGroups();

    bool initSwapChain();
    void releaseSwapChain();

    bool initDepthBuffer();
    void releaseDepthBuffer();

    void resizeWindow(int new_width, int new_height);

    void updateProjectionMatrix();
    void updateViewMatrix();

    void handleInput();

private:
    int m_screenWidth = 640;
    int m_screenHeight = 480;

    bool m_isRunning = true;

    struct UniformData {
        glm::mat4x4 model_matrix;
        glm::mat4x4 view_matrix;
        glm::mat4x4 projection_matrix;
        glm::vec3 camera_world_position;
        float time;
        float padding[3]; // need to chunk into 4x4x4 sections (4x4 floats)
    };
    UniformData m_uniformData;

    struct Camera {
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec2 angles = { 0.0, 0.0 };
        
        float zoom = -3.5;
    };
    Camera m_camera;

    struct DragState {
		bool active = false;
		glm::vec2 startMouse;
		Camera startCameraState;
		float sensitivity = 0.01f;
		float scrollSensitivity = 0.1f;

		glm::vec2 velocity = {0.0, 0.0};
		glm::vec2 previousDelta;
		float inertia = 0.9f;
	};
    DragState m_dragState;

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

    std::vector<bool> m_keys;
};