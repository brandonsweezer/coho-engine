#pragma once
#include "MeshBuilder.h"
#include "../ecs/components/Mesh.h"
#include "../constants.h"
#include <iostream>
#include <glm/glm.hpp>
using vec3 = glm::vec3;
using vec2 = glm::vec2;

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
    std::cout << "generating top cap" << std::endl;
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

    std::cout << "generating middle section" << std::endl;
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

    std::cout << "generating bottom cap" << std::endl;
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

    std::cout << "creating mesh" << std::endl;
    std::shared_ptr<Mesh> uvSphereMesh = std::make_shared<Mesh>();
    uvSphereMesh->setVertexData(vertices);

    std::cout << "returning" << std::endl;
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