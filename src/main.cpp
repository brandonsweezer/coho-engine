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
#include "utilities/MeshBuilder.h"

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

int main() {
    Engine engine;

    std::cout << "initializing models" << std::endl;

    std::shared_ptr<Mesh> skyMesh = MeshBuilder::createUVSphere(10, 10, 1, true);
    std::shared_ptr<Entity> sky = std::make_shared<Entity>();
    sky->addComponent<MeshComponent>();
    sky->getComponent<MeshComponent>()->mesh.reset();
    sky->getComponent<MeshComponent>()->mesh = skyMesh;
    sky->addComponent<TransformComponent>();
    engine.entityManager->setSky(sky, engine.renderer);

    std::vector<Mesh::VertexData> vd;
    ResourceLoader::loadObj(RESOURCE_DIR, "fourareen.obj", vd);
    
    std::shared_ptr<Entity> boat = std::make_shared<Entity>();
    boat->instanceCount = 1000;
    boat->addComponent<MeshComponent>();
    boat->getComponent<MeshComponent>()->mesh->setVertexData(vd);
    boat->addComponent<TransformComponent>();
    boat->getComponent<TransformComponent>()->transform->setPosition(vec3(-5, -5, -5));
    engine.entityManager->addEntity(boat, engine.renderer);

    for (int i = 0; i < 10; ++i) {
        std::cout << i << std::endl;
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {
                std::shared_ptr<Entity> instance = std::make_shared<Entity>();
                instance->addComponent<InstanceComponent>();
                instance->getComponent<InstanceComponent>()->prototype = boat;
                instance->addComponent<TransformComponent>();
                vec3 pos = vec3(i - 5, j - 5, k - 5);
                instance->getComponent<TransformComponent>()->transform->setPosition(pos);
                // instance->getComponent<TransformComponent>()->transform->setScale(vec3(0.2));
                engine.entityManager->addEntity(instance, engine.renderer);
            }
        }
    }
    

    // todo: second render pass + render sky box.
    engine.start();

    return 1;
}