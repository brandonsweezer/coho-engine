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

    engine.start();

    return 1;
}