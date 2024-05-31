#include "MeshBuilder.h"
#include "VertexDataCalculations.h"
#include "../ecs/components/Mesh.h"
#include "../constants.h"
#include <iostream>
#include <glm/glm.hpp>
using vec3 = glm::vec3;
using vec2 = glm::vec2;

std::shared_ptr<Mesh> MeshBuilder::createCube(int size, bool inFacing) {
    float halfsize = (float)(size) / 2.0;
    std::vector<Mesh::VertexData> vertices;
    // Define the 8 unique vertices of the cube
    vec3 positions[] = {
        vec3(-halfsize, -halfsize, -halfsize), // 0: blbot
        vec3(-halfsize, -halfsize,  halfsize), // 1: brbot
        vec3( halfsize, -halfsize, -halfsize), // 2: tlbot
        vec3( halfsize, -halfsize,  halfsize), // 3: trbot
        vec3(-halfsize,  halfsize, -halfsize), // 4: bltop
        vec3(-halfsize,  halfsize,  halfsize), // 5: brtop
        vec3( halfsize,  halfsize, -halfsize), // 6: tltop
        vec3( halfsize,  halfsize,  halfsize)  // 7: trtop
    };

    // Define UV coordinates
    vec2 uvs[] = {
        vec2(0.0f, 0.0f), // 0
        vec2(1.0f, 0.0f), // 1
        vec2(0.0f, 1.0f), // 2
        vec2(1.0f, 1.0f)  // 3
    };

    // Define the normal vectors for each face
    vec3 normals[] = {
        vec3(0, -1, 0),  // Bottom
        vec3(0, 1, 0),   // Top
        vec3(0, 0, 1),   // Front
        vec3(0, 0, -1),  // Back
        vec3(-1, 0, 0),  // Left
        vec3(1, 0, 0)    // Right
    };

    // Define indices for the vertices of each face
    int facesInward[6][6] = {
        {0, 1, 3, 0, 3, 2}, // Bottom
        {4, 7, 5, 4, 6, 7}, // Top
        {1, 5, 7, 1, 7, 3}, // Front
        {0, 2, 6, 0, 6, 4}, // Back
        {0, 4, 5, 0, 5, 1}, // Left
        {2, 3, 7, 2, 7, 6}  // Right
    };

    int facesOutward[6][6] = {
        {0, 2, 3, 0, 3, 1}, // Bottom
        {4, 5, 7, 4, 7, 6}, // Top
        {1, 3, 7, 1, 7, 5}, // Front
        {0, 4, 6, 0, 6, 2}, // Back
        {0, 1, 5, 0, 5, 4}, // Left
        {2, 6, 7, 2, 7, 3}  // Right
    };

    // Define UV indices for each face
    int uvIndicesInward[6][6] = {
        {0, 1, 3, 0, 3, 2}, // Bottom
        {0, 2, 3, 0, 3, 1}, // Top
        {0, 2, 3, 0, 3, 1}, // Front
        {0, 1, 3, 0, 3, 2}, // Back
        {0, 2, 3, 0, 3, 1}, // Left
        {0, 1, 3, 0, 3, 2}  // Right
    };

    int uvIndicesOutward[6][6] = {
        {0, 2, 3, 0, 3, 1}, // Bottom
        {0, 2, 3, 0, 3, 1}, // Top
        {0, 1, 3, 0, 3, 2}, // Front
        {0, 2, 3, 0, 3, 1}, // Back
        {0, 1, 3, 0, 3, 2}, // Left
        {0, 2, 3, 0, 3, 1}  // Right
    };

    // Populate the vertices vector
    for (int i = 0; i < 6; ++i) { // Iterate through each face
        for (int j = 0; j < 6; ++j) { // Each face has 6 vertices (2 triangles)
            int vertexIndex = facesOutward[i][j];
            int uvIndex = uvIndicesOutward[i][j];
            if (inFacing) {
                vertexIndex = facesInward[i][j];
                uvIndex = uvIndicesInward[i][j];
            }
            Mesh::VertexData vertex;
            vertex.position = positions[vertexIndex];
            vertex.normal = normals[i];
            vertex.uv = uvs[uvIndex];
            vertices.push_back(vertex);
        }
    }

    VertexDataCalculations::vertexDataCalculations(vertices);

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->setVertexData(vertices);
    return mesh;
}


std::shared_ptr<Mesh> MeshBuilder::createUVSphere(int rows, int columns, float radius, bool inFacing) {
    int NumVerticesTopStrip = 3 * columns;
    int NumVerticesRegularStrip = 6 * columns;
    int numVertices = NumVerticesTopStrip + (rows - 1) * NumVerticesRegularStrip;

    std::vector<Mesh::VertexData> vertices(numVertices * 2);

    float pitchAngle = 90.0f / (float)rows;
    float headingAngle = 360.0f / (float)columns;
    vec3 apex(0.0f, radius, 0.0f);

    float pitch = -90.0f;
    int i = 0;
    // generate top cap
    for (float heading = 0.0f; heading < 360.0f; heading += headingAngle) {
        Mesh::VertexData v0;
        v0.position = apex;
        v0.normal = apex;
        v0.uv = uvFromSphericalCoords(pitch, heading);
        v0.color = vec3(1 - uvFromSphericalCoords(pitch, heading).y);

        vec3 pos1 = positionFromSphericalCoords(radius, pitch + pitchAngle, heading);
        Mesh::VertexData v1;
        v1.position = pos1;
        v1.normal = pos1;
        v1.uv = uvFromSphericalCoords(pitch + pitchAngle, heading);
        v1.color = vec3(1 - uvFromSphericalCoords(pitch + pitchAngle, heading).y);

        vec3 pos2 = positionFromSphericalCoords(radius, pitch + pitchAngle, heading + headingAngle);
        Mesh::VertexData v2;
        v2.position = pos2;
        v2.normal = pos2;
        v2.uv = uvFromSphericalCoords(pitch + pitchAngle, heading + headingAngle);
        v2.color = vec3(1 - uvFromSphericalCoords(pitch + pitchAngle, heading + headingAngle).y);

        
        if (inFacing) {
            vertices[i++] = v0;
            vertices[i++] = v2;
            vertices[i++] = v1;
        } else {
            vertices[i++] = v0;
            vertices[i++] = v1;
            vertices[i++] = v2;
        }
    }

    // generate top hemisphere
    for (pitch = -90.0f + pitchAngle; pitch < 90.0f - pitchAngle; pitch += pitchAngle) {
        for (float heading = 0.0f; heading < 360.0f; heading += headingAngle) {
            vec3 pos0 = positionFromSphericalCoords(radius, pitch, heading);
            Mesh::VertexData v0;
            v0.position = pos0;
            v0.normal = pos0;
            v0.uv = uvFromSphericalCoords(pitch, heading);
            v0.color = vec3(1 - uvFromSphericalCoords(pitch, heading).y);

            vec3 pos1 = positionFromSphericalCoords(radius, pitch + pitchAngle, heading);
            Mesh::VertexData v1;
            v1.position = pos1;
            v1.normal = pos1;
            v1.uv = uvFromSphericalCoords(pitch + pitchAngle, heading);
            v1.color = vec3(1 - uvFromSphericalCoords(pitch + pitchAngle, heading).y);

            vec3 pos2 = positionFromSphericalCoords(radius, pitch, heading + headingAngle);
            Mesh::VertexData v2;
            v2.position = pos2;
            v2.normal = pos2;
            v2.uv = uvFromSphericalCoords(pitch, heading + headingAngle);
            v2.color = vec3(1 - uvFromSphericalCoords(pitch, heading + headingAngle).y);

            vec3 pos3 = positionFromSphericalCoords(radius, pitch + pitchAngle, heading + headingAngle);
            Mesh::VertexData v3;
            v3.position = pos3;
            v3.normal = pos3;
            v3.uv = uvFromSphericalCoords(pitch + pitchAngle, heading + headingAngle);
            v3.color = vec3(1 - uvFromSphericalCoords(pitch + pitchAngle, heading + headingAngle).y);

            if (inFacing) {
                vertices[i++] = v0;
                vertices[i++] = v2;
                vertices[i++] = v1;

                vertices[i++] = v1;
                vertices[i++] = v2;
                vertices[i++] = v3;
            } else {
                vertices[i++] = v0;
                vertices[i++] = v1;
                vertices[i++] = v2;

                vertices[i++] = v1;
                vertices[i++] = v3;
                vertices[i++] = v2;
            }
            

            
        }
    }

    // generate bottom cap
    pitch = 90.0f;
    apex = vec3(0.0, -radius, 0.0);
    for (float heading = 0.0f; heading < 360.0f; heading += headingAngle) {
        Mesh::VertexData v0;
        v0.position = apex;
        v0.normal = apex;
        v0.uv = uvFromSphericalCoords(pitch, heading);
        v0.color = vec3(1 - uvFromSphericalCoords(pitch, heading).y);

        vec3 pos1 = positionFromSphericalCoords(radius, pitch - pitchAngle, heading);
        Mesh::VertexData v1;
        v1.position = pos1;
        v1.normal = pos1;
        v1.uv = uvFromSphericalCoords(pitch - pitchAngle, heading);
        v1.color = vec3(1 - uvFromSphericalCoords(pitch - pitchAngle, heading).y);

        vec3 pos2 = positionFromSphericalCoords(radius, pitch - pitchAngle, heading - headingAngle);
        Mesh::VertexData v2;
        v2.position = pos2;
        v2.normal = pos2;
        v2.uv = uvFromSphericalCoords(pitch - pitchAngle, heading - headingAngle);
        v2.color = vec3(1 - uvFromSphericalCoords(pitch - pitchAngle, heading - headingAngle).y);
        
        if (inFacing) {
            vertices[i++] = v0;
            vertices[i++] = v2;
            vertices[i++] = v1;
        } else {
            vertices[i++] = v0;
            vertices[i++] = v1;
            vertices[i++] = v2;
        }
    }
    VertexDataCalculations::vertexDataCalculations(vertices);

    std::shared_ptr<Mesh> uvSphereMesh = std::make_shared<Mesh>();
    uvSphereMesh->setVertexData(vertices);

    return uvSphereMesh;
}

vec3 MeshBuilder::positionFromSphericalCoords(float radius, float pitch, float heading) {
    return vec3(
        radius * cosf(glm::radians(pitch)) * sinf(glm::radians(heading)),
        -radius * sinf(glm::radians(pitch)),
        radius * cosf(glm::radians(pitch)) * cosf(glm::radians(heading))
    );
}


vec2 MeshBuilder::uvFromSphericalCoords(float pitch, float heading) {
    // heading goes from 0 to 360
    float u = heading / 360.0;
    // pitch goes from -90 to 90
    float v = ((pitch / 90.0) + 1.0) / 2.0;
    return vec2(u, v);
}