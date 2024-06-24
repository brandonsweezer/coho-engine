#pragma once
#include "webgpu/webgpu.hpp"

#include "gpu/RenderModule.h"
#include "ecs/EntityManager.h"
#include "input/InputManager.h"
#include "terrain/TerrainManager.h"
#include "gpu/ComputeModule.h"
#include "noise/RandomNumberGenerator.h"

class Engine {
public:
    Engine();
    ~Engine();

    void tick();
    void update();
    void draw();

    void start();
    bool isRunning();
    void pause();
    void shutdown();

    uint32_t randomInt(uint32_t max);

    std::shared_ptr<RenderModule> renderModule = nullptr;
    std::shared_ptr<EntityManager> entityManager = nullptr;
    std::shared_ptr<InputManager> inputManager = nullptr;
    std::shared_ptr<TerrainManager> terrainManager = nullptr;
    std::shared_ptr<ComputeModule> computeModule = nullptr;
private:
    void handleInput();
    void updateCamera();

    void setupBindings();
    void wrapCursor(SDL_Event e);

    bool initGPU();
private:
    RandomNumberGenerator m_random;
    bool m_isRunning = false;
    bool m_isSimulating = false;
    bool m_isDrawing = false;

    float m_time = 0.0;
    float m_deltaTime = 0.0;

    InputManager::MouseState m_lastMouseState;
    float m_sensitivity = 0.05;
    float m_rotationSensitivity = 0.001;
    float m_moveSpeed = 25.0;

    std::shared_ptr<wgpu::Device> m_device = nullptr;
    std::unique_ptr<wgpu::ErrorCallback> m_deviceErrorCallback;
    wgpu::Instance m_instance = nullptr;
    SDL_Window* m_window = nullptr;
    std::shared_ptr<wgpu::Surface> m_surface;

    wgpu::TextureFormat m_preferredFormat = wgpu::TextureFormat::BGRA8Unorm;
    int m_screenWidth = 720;
    int m_screenHeight = 480;
};