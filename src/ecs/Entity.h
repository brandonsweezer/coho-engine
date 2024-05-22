#pragma once

#include "components/Components.h"
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class Entity {
private:
    std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;
    int m_id;

public:
    Entity();
    ~Entity();

    template <typename T>
    std::shared_ptr<T> addComponent();

    template <typename T>
    std::shared_ptr<T> getComponent();

    template <typename T>
    bool hasComponent();

    int getId();
    void setId(int id);
};

template <typename T>
std::shared_ptr<T> Entity::addComponent() {
    auto typeIndex = std::type_index(typeid(T));
    std::shared_ptr<T> component = std::make_shared<T>();

    m_components[typeIndex] = component;
    return component;
}

template <typename T>
std::shared_ptr<T> Entity::getComponent() {
    auto typeIndex = std::type_index(typeid(T));
    auto it = m_components.find(typeIndex);
    if (it != m_components.end()) {
        return std::dynamic_pointer_cast<T>(it->second);
    }
    return nullptr;
}

template <typename T>
bool Entity::hasComponent() {
    auto typeIndex = std::type_index(typeid(T));
    return m_components.find(typeIndex) != m_components.end();
}