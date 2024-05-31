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
#include "utilities/io/ExportObj.h"

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

int main() {
    Engine engine;

    // making sky material
    std::shared_ptr<Texture> skyTexture = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "autumn_park_4k.jpg"));
    std::shared_ptr<Material> skyMaterial = std::make_shared<Material>();
    skyMaterial->name = "sky";
    skyMaterial->baseColor = glm::vec3(1.0);
    skyMaterial->diffuseTexture = skyTexture;
    skyMaterial->normalTexture = skyTexture;
    skyMaterial->roughness = 0.5;

    std::shared_ptr<Texture> brickDiffuseTex = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "brick_diffuse.jpg"));
    std::shared_ptr<Texture> brickNormalTex = std::make_shared<Texture>(ResourceLoader::loadTexture(RESOURCE_DIR, "brick_normal.png"));

    // // making brick material
    // std::shared_ptr<Material> brickMaterial = std::make_shared<Material>();
    // brickMaterial->name = "brick";
    // brickMaterial->baseColor = glm::vec3(1.0);
    // brickMaterial->diffuseTexture = brickDiffuseTex;
    // brickMaterial->normalTexture = brickNormalTex;
    // brickMaterial->roughness = 0.5;

    // making sky
    std::shared_ptr<Entity> sky = std::make_shared<Entity>();
    sky->addComponent<TransformComponent>();
    auto meshComponent3 = sky->addComponent<MeshComponent>();
    auto skyMesh = MeshBuilder::createUVSphere(10, 10, 1, true);
    meshComponent3->mesh = skyMesh;
    auto materialComponent3 = sky->addComponent<MaterialComponent>();
    materialComponent3->material = skyMaterial;

    engine.entityManager->setSky(sky, engine.renderer);

    std::vector<VertexData> vd;
    ResourceLoader::loadObj(RESOURCE_DIR, "cube2.obj", vd);
    std::shared_ptr<Entity> cube = std::make_shared<Entity>();
    auto cubeTransform = cube->addComponent<TransformComponent>();
    cubeTransform->transform->setPosition(vec3(-8.0, 0.0, 0.0));
    cubeTransform->transform->setScale(vec3(4.0));
    auto cubeMesh = cube->addComponent<MeshComponent>();
    cubeMesh->mesh->setVertexData(vd);
    engine.entityManager->addEntity(cube, engine.renderer);

    std::shared_ptr<Entity> cube0 = std::make_shared<Entity>();
    auto cubeTransform0 = cube0->addComponent<TransformComponent>();
    cubeTransform0->transform->setPosition(vec3(-4.0, 0.0, 0.0));
    cubeTransform0->transform->setScale(vec3(4.0));
    auto cubeMesh0 = cube0->addComponent<MeshComponent>();
    cubeMesh0->mesh->setVertexData(vd);
    auto cubeMat0 = cube0->addComponent<MaterialComponent>();
    cubeMat0->material->baseColor = vec3(0.662, 0.294, 0.137);
    cubeMat0->material->name = "non-textured material";

    std::shared_ptr<Entity> cube1 = std::make_shared<Entity>();
    auto cubeTransform1 = cube1->addComponent<TransformComponent>();
    cubeTransform1->transform->setPosition(vec3(0, 0, 0.0));
    cubeTransform1->transform->setScale(vec3(4));
    auto cubeMesh1 = cube1->addComponent<MeshComponent>();
    cubeMesh1->mesh->setVertexData(vd);
    auto cubeMat1 = cube1->addComponent<MaterialComponent>();
    cubeMat1->material->baseColor = vec3(0.662, 0.294, 0.137);
    cubeMat1->material->name = "textured material";
    cubeMat1->material->diffuseTexture = brickDiffuseTex;

    std::shared_ptr<Entity> cube2 = std::make_shared<Entity>();
    auto cubeTransform2 = cube2->addComponent<TransformComponent>();
    cubeTransform2->transform->setPosition(vec3(4.0, 0, 0.0));
    cubeTransform2->transform->setScale(vec3(4.0));
    auto cubeMesh2 = cube2->addComponent<MeshComponent>();
    cubeMesh2->mesh->setVertexData(vd);
    auto cubeMat2 = cube2->addComponent<MaterialComponent>();
    cubeMat2->material->baseColor = vec3(0.662, 0.294, 0.137);
    cubeMat2->material->name = "normaled material";
    cubeMat2->material->normalTexture = brickNormalTex;

    std::shared_ptr<Entity> cube3 = std::make_shared<Entity>();
    auto cubeTransform3 = cube3->addComponent<TransformComponent>();
    cubeTransform3->transform->setPosition(vec3(8.0, 0.0, 0.0));
    cubeTransform3->transform->setScale(vec3(4.0));
    auto cubeMesh3 = cube3->addComponent<MeshComponent>();
    cubeMesh3->mesh->setVertexData(vd);
    auto cubeMat3 = cube3->addComponent<MaterialComponent>();
    cubeMat3->material->baseColor = vec3(0.662, 0.294, 0.137);
    cubeMat3->material->name = "textured & normaled material";
    cubeMat3->material->diffuseTexture = brickDiffuseTex;
    cubeMat3->material->normalTexture = brickNormalTex;

    engine.entityManager->addEntity(cube0, engine.renderer);
    engine.entityManager->addEntity(cube1, engine.renderer);
    engine.entityManager->addEntity(cube2, engine.renderer);
    engine.entityManager->addEntity(cube3, engine.renderer);



    engine.start();

    return 1;
}