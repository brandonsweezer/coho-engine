#pragma once
#include <glm/glm.hpp>
#include "../ecs/components/Mesh.h"

class VertexDataCalculations {
public:
    static bool vertexDataCalculations(std::vector<Mesh::VertexData>& vertexData);
    static glm::mat3x3 calculateTBN(Mesh::VertexData v0, Mesh::VertexData v1, Mesh::VertexData v2, glm::vec3 expectedN);

};
