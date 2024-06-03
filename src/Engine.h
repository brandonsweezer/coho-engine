#pragma once
#include "renderer/Renderer.h"
#include "ecs/EntityManager.h"
#include "input/InputManager.h"

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
private:
    void handleInput();
    void updateCamera();

    void setupBindings();
private:
    bool m_isRunning = false;
    bool m_isSimulating = false;
    bool m_isDrawing = false;

    float m_time = 0.0;
    float m_deltaTime = 0.0;
};