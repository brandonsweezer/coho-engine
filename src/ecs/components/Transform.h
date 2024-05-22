#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Transform {
public:
    Transform() {
        m_position = glm::vec3(0.0);
        m_rotation = glm::vec3(0.0);
        m_scale = glm::vec3(1.0);
        recalculateTransform();
    }

    glm::vec3 getPosition() {
        return m_transform[0];
    };
    void setPosition(glm::vec3 pos) {
        m_position = pos;
        recalculateTransform();
    };

    glm::vec3 getRotation() {
        return m_transform[1];
    };
    void setRotation(glm::vec3 rot) {
        m_rotation = rot;
        recalculateTransform();
    };

    glm::vec3 getScale() {
        return m_transform[2];
    };
    void setScale(glm::vec3 scale) {
        m_scale = scale;
        recalculateTransform();
    };

    glm::mat4x4 getMatrix() {
        return m_transform;
    };
    void setMatrix(glm::mat3x3 mat) {
        m_transform = mat;
    };
private:
    void recalculateTransform() {
        m_transform = glm::mat4x4(1.0);
        m_transform = glm::translate(m_transform, m_position);
        // todo: rotations
        m_transform = glm::scale(m_transform, m_scale);
    }

    glm::mat4x4 m_transform;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
};