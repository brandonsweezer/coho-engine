#include "Renderer.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "ecs/components/TransformComponent.h"
#include <iostream>
#include <memory>

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

int main() {
    Renderer renderer;
    if (!renderer.init()) {
        std::cerr << "Could not initialize renderer" << std::endl;
        exit(-1);
    }

    EntityManager entityManager;
    std::shared_ptr<Entity> boat1 = std::make_shared<Entity>();
    std::shared_ptr<Entity> boat2 = std::make_shared<Entity>();
    boat1->addComponent<TransformComponent>();
    boat1->getComponent<TransformComponent>()->transform->setPosition(glm::vec3(0.0, 2.0, 0.0));
    boat2->addComponent<TransformComponent>();
    boat2->getComponent<TransformComponent>()->transform->setPosition(glm::vec3(0.0, -2.0, 0.0));

    entityManager.addEntity(boat1);
    entityManager.addEntity(boat2);
    std::vector<std::shared_ptr<Entity>> entities = entityManager.getAllEntities();
    std::vector<Renderer::ModelData> modelData;
    for (auto entity: entities) {
        glm::mat4x4 transform = entity->getComponent<TransformComponent>()->transform->getMatrix();
        Renderer::ModelData model;
        model.transform = transform;
        modelData.push_back(model);
    }
    renderer.writeModelBuffer(modelData, 0);

    while (renderer.isRunning()) {
        renderer.onFrame(entities);
    }

    renderer.terminate();
    return 1;
}