#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
class Mesh {
public:
    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };

    void setVertexData(std::vector<VertexData> data) {
        m_vertexData = data;
        m_vertexCount = (uint32_t)data.size();
        m_size = (uint32_t)(data.size() * sizeof(VertexData));
    }

    void setIndexData(std::vector<uint32_t> data) {
        m_indexData = data;
        m_indexCount = (uint32_t)data.size();
        isIndexed = true;
    }

    std::vector<uint32_t> getIndexData() {
        return m_indexData;
    }

    uint32_t getVertexBufferOffset() {
        return m_vertexBufferOffset;
    }
    void setVertexBufferOffset(uint32_t offset) {
        m_vertexBufferOffset = offset;
    }

    uint32_t getIndexBufferOffset() {
        return m_indexBufferOffset;
    }
    void setIndexBufferOffset(uint32_t offset) {
        m_indexBufferOffset = offset;
    }
    uint32_t getIndexCount() {
        return m_indexCount;
    }

    uint32_t getSize() {
        return m_size;
    }

    uint32_t getVertexCount() {
        return m_vertexCount;
    }
public:
    bool isIndexed = false;
    std::vector<VertexData> m_vertexData;

private:
    std::vector<uint32_t> m_indexData;
    uint32_t m_indexCount = 0;
    uint32_t m_vertexBufferOffset;
    uint32_t m_indexBufferOffset;
    uint32_t m_vertexCount = 0;
    uint32_t m_size;
};