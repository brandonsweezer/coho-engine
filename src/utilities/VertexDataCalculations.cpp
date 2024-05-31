
#include "VertexDataCalculations.h"
#include <glm/glm.hpp>
#include "../ecs/components/Mesh.h"

using mat3x3 = glm::mat3x3;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using VertexData = Mesh::VertexData;

mat3x3 VertexDataCalculations::calculateTBN(VertexData v0, VertexData v1, VertexData v2, vec3 expectedN) {
    vec3 e1 = v1.position - v0.position;
    vec3 e2 = v2.position - v0.position;

    vec2 uv1 = v1.uv - v0.uv;
    vec2 uv2 = v2.uv - v0.uv;

    vec3 T = glm::normalize(e1 * uv2.y - e2 * uv1.y);
    vec3 B = glm::normalize(e2 * uv1.x - e1 * uv2.x);
    vec3 N = glm::cross(T,B);

    if (dot(N, expectedN) < 0.0) {
        T = -T;
        B = -B;
        N = -N;
    }

    // orthonormalize
    N = expectedN;
    T = glm::normalize(T - dot(T, N) * N);
    B = glm::cross(N, T);

    return mat3x3(T,B,N);

}

bool VertexDataCalculations::vertexDataCalculations(std::vector<VertexData>& vertexData) {
    size_t triangleCount = vertexData.size() / 3;
    for (int i = 0; i < triangleCount; ++i) {
        // calculate tangent and bitangent
        VertexData& v0 = vertexData[3*i + 0];
        VertexData& v1 = vertexData[3*i + 1];
        VertexData& v2 = vertexData[3*i + 2];
        
        glm::mat3x3 TBN0 = calculateTBN(v0, v1, v2, v0.normal);
        v0.tangent = TBN0[0];
        v0.bitangent = TBN0[1];
        v0.normal = TBN0[2];

        glm::mat3x3 TBN1 = calculateTBN(v0, v1, v2, v1.normal);
        v1.tangent = TBN1[0];
        v1.bitangent = TBN1[1];
        v1.normal = TBN1[2];

        glm::mat3x3 TBN2 = calculateTBN(v0, v1, v2, v2.normal);
        v2.tangent = TBN2[0];
        v2.bitangent = TBN2[1];
        v2.normal = TBN2[2];

    }

    return true;
}