#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Mesh {
public:
    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
        uint32_t modelID;
    };

    std::vector<VertexData> getVertexData() { return m_vertexData; };
    void setVertexData(std::vector<VertexData> data) {
        m_vertexData = data;
        m_vertexCount = (uint32_t)data.size();
        m_size = (uint32_t)(data.size() * sizeof(VertexData));
    }

    uint32_t getVertexBufferOffset() {
        return m_vertexBufferOffset;
    }
    void setVertexBufferOffset(uint32_t offset) {
        m_vertexBufferOffset = offset;
    }

    uint32_t getSize() {
        return m_size;
    }

    uint32_t getVertexCount() {
        return m_vertexCount;
    }

private:
    std::vector<VertexData> m_vertexData;
    uint32_t m_vertexBufferOffset;
    uint32_t m_vertexCount = 0;
    uint32_t m_size;
};