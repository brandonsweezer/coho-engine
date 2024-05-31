#include "Engine.h"
#include "ResourceLoader.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "ecs/components/InstanceComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/MaterialComponent.h"
#include "ecs/components/Material.h"
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

    // making sky material
    std::shared_ptr<Texture> skyTexture = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "quarry_cloudy.jpg"));
    std::shared_ptr<Material> skyMaterial = std::make_shared<Material>();
    skyMaterial->name = "sky";
    skyMaterial->baseColor = glm::vec3(1.0);
    skyMaterial->diffuseTexture = skyTexture;
    skyMaterial->normalTexture = skyTexture;
    skyMaterial->roughness = 0.5;

    std::shared_ptr<Texture> brickDiffuseTex = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "brick_diffuse.jpg"));
    std::shared_ptr<Texture> brickNormalTex = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "brick_normal.png"));

    // making brick material
    std::shared_ptr<Material> brickMaterial = std::make_shared<Material>();
    brickMaterial->name = "brick";
    brickMaterial->baseColor = glm::vec3(1.0);
    brickMaterial->diffuseTexture = brickDiffuseTex;
    brickMaterial->normalTexture = brickNormalTex;
    brickMaterial->roughness = 0.5;

    // making first sphere
    std::shared_ptr<Entity> sphere1 = std::make_shared<Entity>();
    sphere1->addComponent<TransformComponent>();
    auto meshComponent = sphere1->addComponent<MeshComponent>();
    auto sphereMesh = MeshBuilder::createUVSphere(10, 10, 2);
    meshComponent->mesh = sphereMesh;
    auto materialComponent = sphere1->addComponent<MaterialComponent>();
    materialComponent->material = brickMaterial;

    engine.entityManager->addEntity(sphere1, engine.renderer);

    // making second sphere
    std::shared_ptr<Entity> sphere2 = std::make_shared<Entity>();
    auto transform2 = sphere2->addComponent<TransformComponent>();
    transform2->transform->setPosition(glm::vec3(4.0, 0.0, 0.0));
    auto meshComponent2 = sphere2->addComponent<MeshComponent>();
    meshComponent2->mesh = sphereMesh;
    auto materialComponent2 = sphere2->addComponent<MaterialComponent>();
    materialComponent2->material = brickMaterial;

    engine.entityManager->addEntity(sphere2, engine.renderer);

    // making sky
    std::shared_ptr<Entity> sky = std::make_shared<Entity>();
    sky->addComponent<TransformComponent>();
    auto meshComponent3 = sky->addComponent<MeshComponent>();
    auto skyMesh = MeshBuilder::createUVSphere(10, 10, 1, true);
    meshComponent3->mesh = skyMesh;
    auto materialComponent3 = sky->addComponent<MaterialComponent>();
    materialComponent3->material = skyMaterial;

    engine.entityManager->setSky(sky, engine.renderer);

    std::shared_ptr<Entity> cube = std::make_shared<Entity>();
    auto cubeTransform = cube->addComponent<TransformComponent>();
    cubeTransform->transform->setPosition(vec3(-4.0, 0.0, 0.0));
    auto cubeMesh = cube->addComponent<MeshComponent>();
    cubeMesh->mesh = MeshBuilder::createCube(1.0);
    auto cubeMat = cube->addComponent<MaterialComponent>();
    cubeMat->material = brickMaterial;
    // cubeMat->material->baseColor = vec3(1.0, 0.0, 0.0);
    // cubeMat->material->name = "non-textured material";
    engine.entityManager->addEntity(cube, engine.renderer);


    engine.start();

    return 1;
}