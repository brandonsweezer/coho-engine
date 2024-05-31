#pragma once
#include "Engine.h"
#include "constants.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
using vec2 = glm::vec2;

Engine::Engine() {
    std::cout << "initializing SDL" << std::endl;
    SDL_SetMainReady();
    renderer = std::make_shared<Renderer>();
    entityManager = std::make_shared<EntityManager>();
    inputManager = std::make_shared<InputManager>();

    entityManager->addDefaultMaterial(renderer);
}

Engine::~Engine() {
    renderer.reset();
    entityManager.reset();
    inputManager.reset();
}

void Engine::start() {
    renderer->startup();
    m_isRunning = true;
    m_isDrawing = true;
    m_isSimulating = true;
    while (m_isRunning) {
        tick();
    }
}

void Engine::tick() {
    handleInput();
    update();
    draw();
}

void Engine::update() {
    if (!m_isSimulating) return;
}

void Engine::draw() {
    if (!m_isDrawing) return;

    float time = SDL_GetTicks64() / 1000.0f;
    renderer->onFrame(entityManager->getRenderableEntities(), entityManager->getSky(), time);
}

void Engine::shutdown() {
    m_isRunning = false;
}

bool Engine::isRunning() {
    return m_isRunning;
}

void Engine::handleInput() {
    if (renderer->m_dragState.active) {
        int xpos, ypos;
        SDL_GetMouseState(&xpos, &ypos);
        vec2 currentMouse = vec2((float)xpos, (float)ypos);
		vec2 delta = (currentMouse - renderer->m_dragState.startMouse) * renderer->m_dragState.sensitivity;
		renderer->m_camera.angles = renderer->m_dragState.startCameraState.angles + delta;
		// Clamp to avoid going too far when orbitting up/down
		renderer->m_camera.angles.y = glm::clamp(renderer->m_camera.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);

		renderer->m_dragState.velocity = delta - renderer->m_dragState.previousDelta;
		renderer->m_dragState.previousDelta = delta;
        renderer->updateViewMatrix();
    }

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                shutdown();
                break;
            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_RESIZED:
                        std::cout << "window resized!!" << std::endl;
                        renderer->resizeWindow(e.window.data1, e.window.data2);
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                renderer->m_dragState.active = true;
                renderer->m_dragState.startCameraState = renderer->m_camera;
                int xpos, ypos;
                SDL_GetMouseState(&xpos, &ypos);
                renderer->m_dragState.startMouse = glm::vec2((float)xpos, (float)ypos);
                break;
            case SDL_MOUSEBUTTONUP:
                renderer->m_dragState.active = false;
                break;
            case SDL_MOUSEWHEEL:
                renderer->m_camera.zoom += e.wheel.y * .1f;
                renderer->updateViewMatrix();
                break;
            default:
                break;
        }
    }
}