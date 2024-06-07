#pragma once
#include "renderer/Renderer.h"
#include "ecs/EntityManager.h"
#include "input/InputManager.h"
#include "terrain/TerrainManager.h"

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

    std::shared_ptr<Renderer> renderer = nullptr;
    std::shared_ptr<EntityManager> entityManager = nullptr;
    std::shared_ptr<InputManager> inputManager = nullptr;
    std::shared_ptr<TerrainManager> terrainManager = nullptr;
private:
    void handleInput();
    void updateCamera();

    void setupBindings();
    void wrapCursor(SDL_Event e);
private:
    bool m_isRunning = false;
    bool m_isSimulating = false;
    bool m_isDrawing = false;

    float m_time = 0.0;
    float m_deltaTime = 0.0;

    InputManager::MouseState m_lastMouseState;
    float m_sensitivity = 0.05;
    float m_rotationSensitivity = 0.001;
    float m_moveSpeed = 25.0;
};