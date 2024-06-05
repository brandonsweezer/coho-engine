#pragma once
#include "EventSubscriber.h"
#include "InputEvents.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <iostream>
#include <functional>

class InputManager {
public:
    struct MouseState {
        glm::vec2 pos;
    };

    InputManager();
    ~InputManager();

    void subscribeToEvent(InputEvent eventType, std::function<void()> callback);
    void subscribeToKeyPress(SDL_Keycode keycode, std::function<void()> callback);

    void unsubscribeFromEvent(InputEvent eventType, int id);

    void emitEvent(InputEvent eventType);
    void emitKeyPress(SDL_Keycode keycode);

    void handleInput(SDL_Event e);
    MouseState getMouseState();
    glm::vec2 getMousePos();
    void setKeyUp(SDL_Keycode key);
    void setKeyDown(SDL_Keycode key);

    void handleMousePress(SDL_MouseButtonEvent& b);
    void handleMouseRelease(SDL_MouseButtonEvent& b);
    void handleControllerPress(SDL_ControllerButtonEvent& b);
    void handleControllerRelease(SDL_ControllerButtonEvent& b);
    void handleScroll(SDL_MouseWheelEvent w);

    bool isKeyDown(SDL_Keycode key);
    bool isMouseButtonDown(Uint8 buttonCode);
    bool isControllerButtonDown(Uint8 buttonCode);
    SDL_MouseWheelEvent getScrollInfo();
private:

    std::unordered_map<SDL_Keycode, bool> keyInfo;
    std::unordered_map<Uint8, bool> mouseInfo;
    std::unordered_map<Uint8, bool> controllerInfo;
    SDL_MouseWheelEvent scrollInfo;

    std::shared_ptr<EventSubscriber<InputEvent>> m_inputSubscriber = nullptr;
    std::shared_ptr<EventSubscriber<SDL_Keycode>> m_keyPressSubscriber = nullptr;
};