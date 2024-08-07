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
std::shared_ptr<Mesh> MeshBuilder::createTerrainPatch(float width, float height, int lod) {
    std::vector<Mesh::VertexData> vertices;
    std::vector<uint32_t> indices;
    int patchVertexCount = 0;
    for (int patchX = 0; patchX <= lod; patchX++) { // each patch consists of LOD number of fans in each direction.
        for (int patchZ = 0; patchZ <= lod; patchZ++) {
            for (float x = -0.5; x <= 0.5; x += 0.5) {
                for (float z = -0.5; z <= 0.5; z += 0.5) {
                    Mesh::VertexData vd;
                    float xPos;
                    float zPos;
                    if (lod== 0) {
                        xPos = x * width;
                        zPos = z * height;
                    } else {
                        xPos = (patchX * (width / float(lod))) + (x * (width / float(lod)));
                        zPos = (patchZ * (height / float(lod))) + (z * (height / float(lod)));
                    }
                    
                    vd.position = glm::vec3(xPos, 0, zPos);
                    vd.uv = glm::vec2(xPos / width, zPos / height);
                    vd.normal = glm::vec3(0, 1, 0);
                    vd.color = glm::vec3(1,0,0);
                    
                    vertices.push_back(vd);
                }
            }
            int baseIndex = patchVertexCount;

            // Indices for the triangle fan of the current patch
            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 3);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 0);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 1);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 5);
            indices.push_back(baseIndex + 2);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 8);
            indices.push_back(baseIndex + 5);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 7);
            indices.push_back(baseIndex + 8);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 6);
            indices.push_back(baseIndex + 7);

            indices.push_back(baseIndex + 4);
            indices.push_back(baseIndex + 3);
            indices.push_back(baseIndex + 6);

            patchVertexCount += 9; // 3x3 grid
        }
    }

    VertexDataCalculations::vertexDataCalculations(vertices);
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->setVertexData(vertices);
    mesh->setIndexData(indices);

    return mesh;
}

std::shared_ptr<Mesh> MeshBuilder::createPlane(float width, float height, uint32_t indicesX, uint32_t indicesY) {
    float stepX = width / indicesX;
    float stepY = height / indicesY;

    std::vector<Mesh::VertexData> vertices;
    for (uint32_t y = 0; y <= indicesY; ++y) {
        for (uint32_t x = 0; x <= indicesX; ++x) {
            // Calculate the position of the vertex
            float xPos = x * stepX - (width / 2.0f);
            // float xPos = x * stepX;
            float yPos = 0.0f;
            float zPos = y * stepY - (height / 2.0f);
            // float zPos = y * stepY;

            // Create a vertex with position, uv, and normal data
            Mesh::VertexData vd;
            vd.position = glm::vec3(xPos, yPos, zPos);
            vd.uv = glm::vec2((float)x / (float)indicesX, (float)y / (float)indicesY);
            vd.normal = glm::vec3(0, 1, 0);

            // Add the vertex to the vertices vector
            vertices.push_back(vd);
        }
    }

    std::vector<uint32_t> indices;
    // Generate indices for the plane mesh using triangles
    for (int y = 0; y < (int)indicesY - 1; ++y) {
        for (int x = 0; x < (int)indicesX; ++x) {
            // Calculate the indices for the triangles
            int topLeft = y * ((int)indicesX + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * ((int)indicesX + 1) + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    std::shared_ptr<Mesh> planeMesh = std::make_shared<Mesh>();
    planeMesh->setVertexData(vertices);
    planeMesh->setIndexData(indices);
    return planeMesh;
}

std::shared_ptr<Mesh> MeshBuilder::createQuad() {
    std::vector<Mesh::VertexData> vertices;
    Mesh::VertexData vd0;
    vd0.uv = vec2(0., 0.);
    vd0.position = vec3(-.5, -.5, 0);
    vertices.push_back(vd0);

    Mesh::VertexData vd1;
    vd1.uv = vec2(1., 1.);
    vd1.position = vec3(.5, .5, 0);
    vertices.push_back(vd1);

    Mesh::VertexData vd2;
    vd2.uv = vec2(0., 1.);
    vd2.position = vec3(-.5, .5, 0);
    vertices.push_back(vd2);

    Mesh::VertexData vd3;
    vd3.uv = vec2(1., 0.);
    vd3.position = vec3(.5, -.5, 0);
    vertices.push_back(vd3);

    std::vector<uint32_t> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(1);
    
    std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>();
    quadMesh->setVertexData(vertices);
    quadMesh->setIndexData(indices);
    return quadMesh;
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