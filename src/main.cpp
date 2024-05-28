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

    std::shared_ptr<Entity> sphere = std::make_shared<Entity>();
    sphere->addComponent<MeshComponent>();
    sphere->getComponent<MeshComponent>()->mesh->setVertexData(MeshBuilder::createUVSphere(100,100,10)->getVertexData());
    sphere->addComponent<TransformComponent>();
    engine.entityManager->addEntity(sphere, engine.renderer);

    engine.start();

    return 1;
}