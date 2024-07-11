#pragma once
#include "Engine.h"
#include "constants.h"
#include "input/InputManager.h"
#include "input/InputEvents.h"
#include "gpu/ComputeModule.h"
#include "noise/RandomNumberGenerator.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;

Engine::Engine() {
    std::cout << "initializing SDL {" << std::endl;
    SDL_SetMainReady();
    std::cout << "}" << std::endl;

    std::cout << "initializing GPU {" << std::endl;
    if (!initGPU()) {
        std::cout << "failed to init GPU!" << std::endl;
    }
    std::cout << "}" << std::endl;
    
    std::cout << "initializing renderModule {" << std::endl;
    renderModule = std::make_shared<RenderModule>(m_screenWidth, m_screenHeight, m_device, m_surface);
    std::cout << "}" << std::endl;

    std::cout << "initializing compute module {" << std::endl;
    computeModule = std::make_shared<ComputeModule>();
    std::cout << "}" << std::endl;
    
    std::cout << "initializing entity manager {" << std::endl;
    entityManager = std::make_shared<EntityManager>();
    entityManager->addDefaultMaterial(renderModule);
    std::cout << "}" << std::endl;
    
    std::cout << "initializing input manager {" << std::endl;
    inputManager = std::make_shared<InputManager>();
    std::cout << "}" << std::endl;

    std::cout << "initializing terrainManager {" << std::endl;
    terrainManager = std::make_shared<TerrainManager>(m_device, 1);
    renderModule->addTerrainPipeline(
        terrainManager->getPipeline(),
        terrainManager->getVertexBuffer(),
        terrainManager->getVertexCount(),
        terrainManager->getIndexBuffer(),
        terrainManager->getIndexCount(),
        terrainManager->getUniformBuffer()
        );
    std::cout << "}" << std::endl;

    std::cout << "initializing random number generator {" << std::endl;
    m_random = RandomNumberGenerator(42);
    std::cout << "}" << std::endl;

    std::cout << "setting up bindings {" << std::endl;
    setupBindings();
    std::cout << "}" << std::endl;
}

Engine::~Engine() {
    renderModule.reset();
    entityManager.reset();
    inputManager.reset();
    terrainManager.reset();

    m_surface.reset();
    m_device.reset();

    SDL_RELEASE(m_window);
    SDL_Quit();

    m_instance.release();
}

void Engine::start() {
    std::cout << "starting up engine..." << std::endl;
    renderModule->startup();
    m_isRunning = true;
    m_isDrawing = true;
    m_isSimulating = true;

    auto quad = std::make_shared<Entity>();
    quad->addComponent<TransformComponent>();
    quad->addComponent<MeshComponent>()->mesh = MeshBuilder::createQuad();
    // quad->addComponent<MeshComponent>()->mesh = MeshBuilder::createCube(1);
    entityManager->setQuad(quad, renderModule);

    std::cout << "== vroom vroom ==" << std::endl;
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
    // todo: clamp pitch between -90 and 90 degrees
    auto mouseState = inputManager->getMouseState();
    auto cameraTransform = entityManager->camera->getComponent<TransformComponent>()->transform;
    vec2 pixelDelta = (mouseState.pos - m_lastMouseState.pos);
    vec2 pitchYaw = vec2(-pixelDelta.y, pixelDelta.x) * PI * 2.0f * m_deltaTime * m_sensitivity;
    cameraTransform->rotateEuler(vec3(pitchYaw.x, pitchYaw.y, 0));
    // end first person camera implementation

    renderModule->m_camera.transform = cameraTransform->getMatrix();
    renderModule->m_camera.position = cameraTransform->getPosition();
    renderModule->m_camera.forward = cameraTransform->getForward();
    renderModule->m_camera.up = cameraTransform->getUp();
    renderModule->m_camera.right = cameraTransform->getRight();

    renderModule->updateViewMatrix();
}

void Engine::draw() {
    updateCamera();
    renderModule->onFrame(
        terrainManager->getTerrainPatches(renderModule->m_camera),
        entityManager->getRenderableEntities(),
        entityManager->getSky(),
        entityManager->getQuad(),
        m_time);

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

uint32_t Engine::randomInt(uint32_t max) {
    return m_random.next(max);
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
                        m_screenWidth = e.window.data1;
                        m_screenHeight = e.window.data2;
                        renderModule->resizeWindow(m_screenWidth, m_screenHeight);
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
    vec2 screenDimension = vec2(m_screenWidth, m_screenHeight);
    if (e.motion.x == screenDimension.x - 1) {
        SDL_WarpMouseInWindow(m_window, 1, e.motion.y);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.x == 0) {
        SDL_WarpMouseInWindow(m_window, screenDimension.x - 2, e.motion.y);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.y == screenDimension.y - 1) {
        SDL_WarpMouseInWindow(m_window, e.motion.x, 1);
        m_lastMouseState = inputManager->getMouseState();
    }
    if (e.motion.y == 0) {
        SDL_WarpMouseInWindow(m_window, e.motion.x, screenDimension.y - 2);
        m_lastMouseState = inputManager->getMouseState();
    }
}

bool Engine::initGPU() {
    m_instance = createInstance(InstanceDescriptor{});
    if (!m_instance) return false;

    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return false;
    }
    SDL_SetRelativeMouseMode(SDL_TRUE);

    std::cout << "initializing window" << std::endl;
    m_window = SDL_CreateWindow("Coho", 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_screenWidth,
        m_screenHeight,
        SDL_WINDOW_RESIZABLE);
    if (!m_window) return false;

    std::cout << "initializing surface" << std::endl;
    m_surface = std::make_shared<wgpu::Surface>(SDL_GetWGPUSurface(m_instance, m_window));
    if (!m_surface) return false;

    std::cout << "initializing adapter" << std::endl;
    RequestAdapterOptions adapterOptions;
    adapterOptions.compatibleSurface = *m_surface;
    adapterOptions.forceFallbackAdapter = false;
    adapterOptions.powerPreference = PowerPreference::Undefined;
    Adapter adapter = m_instance.requestAdapter(adapterOptions);
    if (!adapter) return false;

    m_preferredFormat = m_surface->getPreferredFormat(adapter);

    std::cout << "initializing device" << std::endl;
    RequiredLimits requiredLimits;
    requiredLimits.limits.minStorageBufferOffsetAlignment = 64;
    requiredLimits.limits.minUniformBufferOffsetAlignment = 64;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(DefaultPipeline::UniformData);
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxBindingsPerBindGroup = 7;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxVertexAttributes = 7;
    requiredLimits.limits.maxBufferSize = 1000000 * sizeof(DefaultPipeline::ModelData); // 1,000,000 models
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(Mesh::VertexData);
    requiredLimits.limits.maxInterStageShaderComponents = 22;
    requiredLimits.limits.maxStorageBuffersPerShaderStage = 2;
    requiredLimits.limits.maxStorageBufferBindingSize = 1000000 * sizeof(DefaultPipeline::ModelData); // 1 million objects

    requiredLimits.limits.maxTextureDimension1D = 8192;
    requiredLimits.limits.maxTextureDimension2D = 8192;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    requiredLimits.limits.maxSampledTexturesPerShaderStage = 100;
    requiredLimits.limits.maxSamplersPerShaderStage = 2;

    DeviceDescriptor deviceDesc;
    deviceDesc.defaultQueue = QueueDescriptor{};
    deviceDesc.requiredFeatureCount = 2;
    // special conversion for native features
    std::vector<WGPUFeatureName> reqFeatures{
        (WGPUFeatureName)NativeFeature::TextureBindingArray,
        (WGPUFeatureName)NativeFeature::SampledTextureAndStorageBufferArrayNonUniformIndexing
    };
    deviceDesc.requiredFeatures = reqFeatures.data();
    deviceDesc.requiredLimits = &requiredLimits;
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const * message, void * userdata) {
        std::cout << "Device lost! Reason: " << reason << userdata << "(\n" << message << "\n)" << std::endl;
    };

    m_device = std::make_shared<wgpu::Device>(adapter.requestDevice(deviceDesc));
    if (!m_device) return false;
    m_deviceErrorCallback = m_device->setUncapturedErrorCallback([](ErrorType type, char const * message) {
        std::cout << "Device error! " << type << "(\n" << message << "\n)" << std::endl;
    });

    return true;
}