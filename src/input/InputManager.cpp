#include "InputManager.h"
#include "EventSubscriber.h"
#include "InputEvents.h"
#include <SDL2/sdl.h>
#include <glm/glm.hpp>
#include <functional>

InputManager::InputManager() {
    m_inputSubscriber = std::make_shared<EventSubscriber<InputEvent>>();
}

InputManager::~InputManager() {
    m_inputSubscriber.reset();
}

void InputManager::subscribeToEvent(InputEvent eventType, std::function<void()> callback) {
    m_inputSubscriber->subscribe(eventType, callback);
}

void InputManager::unsubscribeFromEvent(InputEvent eventType, int id) {
    m_inputSubscriber->unsubscribe(eventType, id);
}

void InputManager::emitEvent(InputEvent eventType) {
    m_inputSubscriber->emit(eventType);
}

glm::vec2 InputManager::getMousePos() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return glm::vec2(x,y);
}

void InputManager::setKeyDown(SDL_Keycode key) {
    keyInfo[key] = true;
    emitEvent(InputEvent::KEY_DOWN);
}

void InputManager::setKeyUp(SDL_Keycode key) {
    keyInfo[key] = false;
    emitEvent(InputEvent::KEY_UP);
}

// todo: overload with const char *, I want to say isKeyDown("a")
bool InputManager::isKeyDown(SDL_Keycode key) {
    return keyInfo[key];
}

void InputManager::handleMousePress(SDL_MouseButtonEvent& b) {
    mouseInfo[b.button] = true;
    emitEvent(InputEvent::MOUSE_DOWN);
}

void InputManager::handleMouseRelease(SDL_MouseButtonEvent& b) {
    mouseInfo[b.button] = false;
    emitEvent(InputEvent::MOUSE_UP);
}

void InputManager::handleScroll(SDL_MouseWheelEvent w) {
    scrollInfo = w;
    emitEvent(InputEvent::SCROLL);
}

bool InputManager::isMouseButtonDown(uint8_t button) {
    return mouseInfo[button];
}

SDL_MouseWheelEvent InputManager::getScrollInfo() {
    return scrollInfo;
}
