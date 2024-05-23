#include "Renderer.h"
#include "ResourceLoader.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
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

    std::cout << "initializing models" << std::endl;
    for (int i = 0; i < 10; ++i) {
        ResourceLoader::loadObj(RESOURCE_DIR, "fourareen.obj", vd, i); // this sucks doing 10 times, it's the next thing to go!
        std::shared_ptr<Entity> boat = std::make_shared<Entity>();
        
        boat->addComponent<MeshComponent>();
        boat->getComponent<MeshComponent>()->mesh->setVertexData(vd);
        boat->addComponent<TransformComponent>();
        boat->getComponent<TransformComponent>()->transform->setPosition(
            glm::vec3(0.0, (float)i*2 - 10.0, 0.0)
        );
        entityManager.addEntity(boat);

    }
    std::vector<std::shared_ptr<Entity>> entities = entityManager.getAllEntities();
    std::cout << "begin rendering" << std::endl;
    while (renderer->isRunning()) {
        renderer->onFrame(entities);
    }

    renderer->terminate();
    return 1;
}