#include "Renderer.h"
#include "ResourceLoader.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "ecs/components/InstanceComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/Mesh.h"
#include <iostream>
#include <memory>

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

int main() {
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    if (!renderer->init()) {
        std::cerr << "Could not initialize renderer" << std::endl;
        exit(-1);
    }

    EntityManager entityManager(renderer);
    std::vector<VertexData> vd;
    ResourceLoader::loadObj(RESOURCE_DIR, "cube.obj", vd);
    std::cout << "initializing models" << std::endl;

    // create the prototype
    std::shared_ptr<Entity> prototype = std::make_shared<Entity>();
    prototype->addComponent<MeshComponent>();
    prototype->getComponent<MeshComponent>()->mesh->setVertexData(vd);
    prototype->addComponent<TransformComponent>();

    float ypos = 0;
    float xpos = 0;
    float zpos = 0;
    glm::vec3 pos(xpos, ypos, zpos);
    pos = pos * glm::vec3(2.0) - glm::vec3(4.0);
    prototype->getComponent<TransformComponent>()->transform->setPosition(
        pos
    );
    prototype->instanceCount = 1000000;
    entityManager.addEntity(prototype);

    // create instances
    for (int i = 0; i < (1000000 - 1); ++i) {
        std::shared_ptr<Entity> instance = std::make_shared<Entity>();
        
        instance->addComponent<InstanceComponent>();
        instance->getComponent<InstanceComponent>()->prototype = prototype;

        instance->addComponent<TransformComponent>();
        ypos = i / (100 * 100);
        xpos = i % 100;
        zpos = (i / 100) % 100;
        glm::vec3 instancePos = glm::vec3(xpos, ypos, zpos);
        instancePos = instancePos * glm::vec3(2.0) - glm::vec3(100.0);
        instance->getComponent<TransformComponent>()->transform->setPosition(
            instancePos
        );
        instance->getComponent<TransformComponent>()->transform->setScale(
            glm::vec3(0.5)
        );
        entityManager.addEntity(instance);

    }
    std::vector<std::shared_ptr<Entity>> entities = entityManager.getAllEntities();
    std::cout << "begin rendering" << std::endl;
    while (renderer->isRunning()) {
        renderer->onFrame(entities);
    }

    renderer->terminate();
    return 1;
}