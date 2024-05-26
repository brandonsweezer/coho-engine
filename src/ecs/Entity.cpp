#include "Entity.h"

// Constructor definition
Entity::Entity() {
    // default constructor implementation
    instanceCount = 1;
}

// Destructor definition
Entity::~Entity() {
    // destructor implementation
}

void Entity::setId(int id) {
    m_id = id;
}

int Entity::getId() {
    return m_id;
}