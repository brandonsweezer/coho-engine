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
    ResourceLoader::loadObj(RESOURCE_DIR, "fourareen.obj", vd); // this sucks doing 10 times, it's the next thing to go!
    std::cout << "initializing models" << std::endl;
    for (int i = 0; i < 64; ++i) {
        std::shared_ptr<Entity> boat = std::make_shared<Entity>();
        
        boat->addComponent<MeshComponent>();
        boat->getComponent<MeshComponent>()->mesh->setVertexData(vd);
        boat->addComponent<TransformComponent>();

        float ypos = i / 16;
        float xpos = i % 4;
        float zpos = (i / 4) % 4;
        glm::vec3 pos(xpos, ypos, zpos);
        pos = pos * glm::vec3(2.0) - glm::vec3(4.0);
        boat->getComponent<TransformComponent>()->transform->setPosition(
            pos
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