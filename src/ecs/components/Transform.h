#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "../../constants.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

class Transform {
public:
    Transform() {
        m_forward = glm::vec3(0.0, 0.0, 1.0);
        m_right = glm::vec3(1.0, 0.0, 0.0);
        m_up = glm::vec3(0.0, 1.0, 0.0);

        m_position = glm::vec3(0.0);
        m_rotation = glm::quat(0.0, 0.0, 0.0, 0.0);
        m_scale = glm::vec3(1.0);
        recalculateTransform();
    }

    glm::vec3 getPosition() {
        return m_position;
    };
    void setPosition(glm::vec3 pos) {
        m_position = pos;
        recalculateTransform();
    };

    glm::vec3 getRight() {
        return glm::normalize(glm::vec3(m_transform[0][0], m_transform[1][0], m_transform[2][0]));
    }
    glm::vec3 getUp() {
        return glm::normalize(glm::vec3(m_transform[0][1], m_transform[1][1], m_transform[2][1]));
    }
    glm::vec3 getForward() {
        return glm::normalize(glm::vec3(m_transform[0][2], m_transform[1][2], m_transform[2][2]));
    }

    glm::quat getRotation() {
        return m_rotation;
    };

    // accepts vec3 of pitch, yaw, roll in radians
    void setRotation(glm::vec3 rot) {
        m_rotation = eulerToQuaternion(rot);
        recalculateTransform();
    };

    void rotateEuler(glm::vec3 rot) {
        m_transform = glm::rotate(m_transform, rot.x, getRight());
        m_transform = glm::rotate(m_transform, rot.y, glm::vec3(0, 1, 0));
        m_transform = glm::rotate(m_transform, rot.z, getForward());
        m_rotation = getRotationFromTransform();
    }

    // accepts a quaternion
    void setRotation(glm::quat rot) {
        m_rotation = rot;
        recalculateTransform();
    };

    glm::vec3 getScale() {
        return m_scale;
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
        glm::mat4 rotationMatrix = glm::mat4_cast(m_rotation);
        m_transform *= rotationMatrix;
        m_transform = glm::scale(m_transform, m_scale);
    }

    float wrapAngle(float angle, float min, float max) {
        float range = max - min;
        float result = fmod(angle - min, range);
        if (result < 0) {
            result += range;
        }
        return result + min;
    }

    glm::quat eulerToQuaternion(glm::vec3 euler) {
        glm::quat pitchQuat = glm::angleAxis(euler.x, glm::vec3(1, 0, 0));
        glm::quat yawQuat = glm::angleAxis(euler.y, glm::vec3(0, 1, 0));
        glm::quat rollQuat = glm::angleAxis(euler.z, glm::vec3(0, 0, 1));

        // Combine the quaternions in the order of roll, pitch, yaw
        glm::quat quaternion = yawQuat * pitchQuat * rollQuat;
        
        return quaternion;
    }

    glm::quat getRotationFromTransform() {
        glm::mat3 rotationMatrix = glm::mat3(m_transform);
        glm::quat quaternion = glm::quat(rotationMatrix);
        return quaternion;
    }

private:

    glm::mat4x4 m_transform;
    glm::vec3 m_position;
    
    glm::quat m_rotation;
    glm::vec3 m_scale;

    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
};