#include "Engine.h"
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
    Engine engine;

    std::vector<VertexData> vd;
    ResourceLoader::loadObj(RESOURCE_DIR, "cube.obj", vd);
    std::cout << "initializing models" << std::endl;

    // create the prototype
    std::shared_ptr<Entity> prototype = std::make_shared<Entity>();
    prototype->addComponent<MeshComponent>();
    prototype->instanceCount = 1000;
    float cuberoot = std::trunc(std::pow(prototype->instanceCount, 1.0/3.0));
    std::cout << "cube root of " << prototype->instanceCount << ": " << cuberoot << std::endl;

    prototype->getComponent<MeshComponent>()->mesh->setVertexData(vd);
    prototype->addComponent<TransformComponent>();

    float ypos = 0;
    float xpos = 0;
    float zpos = 0;
    glm::vec3 pos(xpos, ypos, zpos);
    pos = pos - glm::vec3(cuberoot);
    prototype->getComponent<TransformComponent>()->transform->setPosition(
        pos
    );
    engine.entityManager->addEntity(prototype, engine.renderer);

    // create instances
    for (int i = 0; i < (prototype->instanceCount - 1); ++i) {
        std::shared_ptr<Entity> instance = std::make_shared<Entity>();
        
        instance->addComponent<InstanceComponent>();
        instance->getComponent<InstanceComponent>()->prototype = prototype;

        instance->addComponent<TransformComponent>();
        ypos = i / (cuberoot * cuberoot);
        xpos = i % (int)std::trunc(cuberoot);
        zpos = (i / (int)cuberoot) % (int)std::trunc(cuberoot);
        glm::vec3 instancePos = glm::vec3(xpos, ypos, zpos);
        instancePos = instancePos - glm::vec3(cuberoot);
        instance->getComponent<TransformComponent>()->transform->setPosition(
            instancePos
        );
        instance->getComponent<TransformComponent>()->transform->setScale(
            glm::vec3(1.0)
        );
        engine.entityManager->addEntity(instance, engine.renderer);

    }

    engine.start();

    return 1;
}