#pragma once
#include "Engine.h"
#include "constants.h"
#include "input/InputManager.h"
#include "input/InputEvents.h"
#include "ecs/components/TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;

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
    
}

void Engine::updateCamera() {
    if (inputManager->isKeyDown(SDLK_w)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getForward() * m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }
    if (inputManager->isKeyDown(SDLK_s)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getForward() * -m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }
    if (inputManager->isKeyDown(SDLK_a)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getRight() * m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }
    if (inputManager->isKeyDown(SDLK_d)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getRight() * -m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }
    if (inputManager->isKeyDown(SDLK_SPACE)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getUp() * m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }
    if (inputManager->isKeyDown(SDLK_LSHIFT)) {
        auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
        glm::vec3 deltaMovement = cameraTransform->getUp() * -m_moveSpeed * m_deltaTime;
        glm::vec3 newPos = cameraTransform->getPosition() + deltaMovement;
        cameraTransform->setPosition(newPos);
    }

    // first person camera implementation
    auto mouseState = inputManager->getMouseState();
    auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
    vec2 pixelDelta = (mouseState.pos - m_lastMouseState.pos);
    vec2 pitchYaw = vec2(-pixelDelta.y, pixelDelta.x) * PI * 2.0f * m_deltaTime * m_sensitivity;
    cameraTransform->rotateEuler(vec3(pitchYaw.x, pitchYaw.y, 0));
    // end first person camera implementation

    renderer->m_camera.transform = cameraTransform->getMatrix();
    renderer->m_camera.position = cameraTransform->getPosition();
    renderer->m_camera.forward = cameraTransform->getForward();
    renderer->m_camera.up = cameraTransform->getUp();
    renderer->m_camera.right = cameraTransform->getRight();

    renderer->updateViewMatrix();
}

void Engine::draw() {
    updateCamera();
    renderer->onFrame(entityManager->getRenderableEntities(), entityManager->getSky(), m_time);

    m_lastMouseState = inputManager->getMouseState();
}

void Engine::shutdown() {
    m_isRunning = false;
}

bool Engine::isRunning() {
    return m_isRunning;
}

void Engine::setupBindings() {

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
                if (SDL_GetRelativeMouseMode()) {
                    inputManager->handleMousePress(e.button);
                } else {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                inputManager->handleMouseRelease(e.button);
                break;
            case SDL_MOUSEWHEEL:
                inputManager->handleScroll(e.wheel);
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
                inputManager->setKeyDown(e.key.keysym.sym);
                break;
            case SDL_KEYUP:
                inputManager->setKeyUp(e.key.keysym.sym);
                break;
            case SDL_MOUSEMOTION:
                if (SDL_GetRelativeMouseMode()) {
                   wrapCursor(e);
                }
                inputManager->emitEvent(InputEvent::MOUSE_MOVE);
            default:
                break;
        }
    }
}

void Engine::wrapCursor(SDL_Event e) {
    vec2 screenDimension = renderer->getScreenDimensions();
    if (e.motion.x == screenDimension.x - 1) {
        SDL_WarpMouseInWindow(renderer->getWindow(), 1, e.motion.y);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.x == 0) {
        SDL_WarpMouseInWindow(renderer->getWindow(), screenDimension.x - 2, e.motion.y);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.y == screenDimension.y - 1) {
        SDL_WarpMouseInWindow(renderer->getWindow(), e.motion.x, 1);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.y == 0) {
        SDL_WarpMouseInWindow(renderer->getWindow(), e.motion.x, screenDimension.y - 2);
        m_lastMouseState = inputManager->getMouseState();
    }
}