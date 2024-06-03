#pragma once
#include "Engine.h"
#include "constants.h"
#include "input/InputManager.h"
#include "input/InputEvents.h"
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
    setupBindings();
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
    float newTime = SDL_GetTicks64() / 1000.0f;
    m_deltaTime = newTime - m_time;
    m_time = newTime;
    
    handleInput();

    if (m_isSimulating) {
        update();
    }

    if (m_isDrawing) {
        draw();
    }
}

void Engine::update() {
    updateCamera();
}

void Engine::updateCamera() {
    if (renderer->m_dragState.active) {
        vec2 currentMouse = inputManager->getMousePos();
		vec2 delta = (currentMouse - renderer->m_dragState.startMouse) * renderer->m_dragState.sensitivity;
		renderer->m_camera.angles = renderer->m_dragState.startCameraState.angles + delta;
		// Clamp to avoid going too far when orbitting up/down
		renderer->m_camera.angles.y = glm::clamp(renderer->m_camera.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);

		renderer->m_dragState.velocity = delta - renderer->m_dragState.previousDelta;
		renderer->m_dragState.previousDelta = delta;
        renderer->updateViewMatrix();
    }
}

void Engine::draw() {
    renderer->onFrame(entityManager->getRenderableEntities(), entityManager->getSky(), m_time);
}

void Engine::shutdown() {
    m_isRunning = false;
}

bool Engine::isRunning() {
    return m_isRunning;
}

void Engine::setupBindings() {
    inputManager->subscribeToEvent(InputEvent::SCROLL, [&]() {
        auto wheel = inputManager->getScrollInfo();
        renderer->m_camera.zoom += wheel.y * .1f;
        renderer->updateViewMatrix();
    });
    inputManager->subscribeToEvent(InputEvent::MOUSE_DOWN, [&]() {
        renderer->m_dragState.active = true;
        renderer->m_dragState.startCameraState = renderer->m_camera;
        renderer->m_dragState.startMouse = inputManager->getMousePos();
    });
    inputManager->subscribeToEvent(InputEvent::MOUSE_UP, [&]() {
        renderer->m_dragState.active = false;
    });

}

void Engine::handleInput() {
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
                inputManager->handleMousePress(e.button);
                break;
            case SDL_MOUSEBUTTONUP:
                inputManager->handleMouseRelease(e.button);
                break;
            case SDL_MOUSEWHEEL:
                inputManager->handleScroll(e.wheel);
                break;
            case SDL_KEYDOWN:
                inputManager->setKeyDown(e.key.keysym.sym);
                break;
            case SDL_KEYUP:
                inputManager->setKeyUp(e.key.keysym.sym);
                break;
            default:
                break;
        }
    }
}