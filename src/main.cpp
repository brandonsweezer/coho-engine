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
#include "ecs/components/Texture.h"
#include <iostream>
#include <memory>
#include "utilities/MeshBuilder.h"
#include "utilities/io/ExportObj.h"

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace coho;

int main() {
    Engine engine;

    // making sky material
    std::shared_ptr<coho::Texture> skyTexture = std::make_shared<coho::Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "textures/autumn_park_4k.jpg"));
    std::shared_ptr<Material> skyMaterial = std::make_shared<Material>();
    skyMaterial->name = "sky";
    skyMaterial->baseColor = glm::vec3(1.0);
    skyMaterial->diffuseTexture = skyTexture;
    skyMaterial->normalTexture = skyTexture;
    skyMaterial->roughness = 0.5;

    std::shared_ptr<coho::Texture> brickDiffuseTex = std::make_shared<coho::Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "textures/brick_diffuse.jpg"));
    std::shared_ptr<coho::Texture> brickNormalTex = std::make_shared<coho::Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "textures/brick_normal.png"));

    // making brick material
    std::shared_ptr<Material> brickMaterial = std::make_shared<Material>();
    brickMaterial->name = "brick";
    brickMaterial->baseColor = glm::vec3(1.0);
    brickMaterial->diffuseTexture = brickDiffuseTex;
    brickMaterial->normalTexture = brickNormalTex;
    brickMaterial->roughness = 0.5;

    // making sky
    std::shared_ptr<Entity> sky = std::make_shared<Entity>();
    sky->addComponent<TransformComponent>();
    auto meshComponent3 = sky->addComponent<MeshComponent>();
    auto skyMesh = MeshBuilder::createUVSphere(10, 10, 10, true);
    meshComponent3->mesh = skyMesh;
    auto materialComponent3 = sky->addComponent<MaterialComponent>();
    materialComponent3->material = skyMaterial;

    engine.entityManager->setSky(sky, engine.renderModule);

    engine.start();

    return 1;
}