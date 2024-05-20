#include "Renderer.h"
#include <iostream>

#define SDL_MAIN_HANDLED
#undef main
#include <SDL2/SDL.h>

int main() {
    Renderer renderer;
    if (!renderer.init()) {
        std::cerr << "Could not initialize renderer" << std::endl;
        exit(-1);
    }

    while (renderer.isRunning()) {
        renderer.onFrame();
    }

    renderer.terminate();
    return 1;
}