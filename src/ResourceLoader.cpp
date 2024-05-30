#pragma once
#include "ResourceLoader.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <webgpu/webgpu.hpp>

#include "ecs/components/Mesh.h"
#include "ecs/components/Texture.h"
#include "ecs/components/Material.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
namespace fs = std::filesystem;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using mat3x3 = glm::mat3x3;

using namespace coho;

Texture ResourceLoader::loadTexture(const std::string& path, const std::string& filename) {
    Texture texture;
    
    int width, height, channels;
    auto data = stbi_load(std::string(path + std::string("/") + filename).c_str(), &width, &height, &channels, 4);

    texture.width = width;
    texture.height = height;
    texture.channels = channels;
    size_t dataSize = width * height * 4;
    texture.pixelData = std::vector<unsigned char>(data, data + dataSize);
    texture.mipLevels = 8;

    stbi_image_free(data);

    return texture;
}

std::string ResourceLoader::loadShaderCode(std::string path) {
    std::ifstream file(path);
	if (!file.is_open()) {
		return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

    return shaderSource;
}

bool ResourceLoader::loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path + std::string("/") + filename, 0);

    if (!warn.empty()) {
        std::cout << "warning: " << warn.c_str() << std::endl;
    }
    if (!err.empty()) {
        std::cout << "err: " << err.c_str() << std::endl;
    }
    if (!ret) {
        std::cout << "failed to load file: " << path << "/" << filename << std::endl;
        return false;
    }

    vertexData.clear();
    for (tinygltf::Mesh mesh : model.meshes) {
        for (tinygltf::Primitive primitive : mesh.primitives) {
            int accessorIndex = primitive.attributes["POSITION"];
            tinygltf::Accessor accessor = model.accessors[accessorIndex];
            tinygltf::BufferView bufferView = model.bufferViews[accessor.bufferView];
            tinygltf::Buffer buffer = model.buffers[bufferView.buffer];
            
        }
    }
    // TODO this doesn't work, and I'd rather spend my time working on adding rendering features rather than parsing files
    return true;
}

bool ResourceLoader::loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData) {
    std::cout << "loading .obj file: " << filename << "from " << path << std::endl;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = path.c_str();

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path + std::string("/") + filename, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return false;
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    // auto& materials = reader.GetMaterials();


    vertexData.clear();
    
    // could be multiple shapes in obj file
    for (auto shape : shapes) {
        // combine all shapes into offset VertexData array
        size_t offset = vertexData.size();
        vertexData.resize(offset + shape.mesh.indices.size());
        for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            tinyobj::index_t it = shape.mesh.indices[i];
            vertexData[offset + i].position = {
                attrib.vertices[3 * it.vertex_index + 0],
                attrib.vertices[3 * it.vertex_index + 1],
                attrib.vertices[3 * it.vertex_index + 2]
            };
            vertexData[offset + i].normal = {
                attrib.normals[3 * it.normal_index + 0],
                attrib.normals[3 * it.normal_index + 1],
                attrib.normals[3 * it.normal_index + 2]
            };
            vertexData[offset + i].color = {
                attrib.colors[3 * it.vertex_index + 0],
                attrib.colors[3 * it.vertex_index + 1],
                attrib.colors[3 * it.vertex_index + 2]
            };
            vertexData[offset + i].uv = {
                attrib.texcoords[2 * it.texcoord_index + 0],
                1 - attrib.texcoords[2 * it.texcoord_index + 1],
            };
        }
        
    }

    bool success = vertexDataCalculations(vertexData);
    if (!success) {
        std::cout << "failed to do vertex calculations" << std::endl;
    }

    std::cout << "done loading " << path << std::endl;

    return true;
}

mat3x3 ResourceLoader::calculateTBN(VertexData v0, VertexData v1, VertexData v2, vec3 expectedN) {
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

bool ResourceLoader::vertexDataCalculations(std::vector<VertexData>& vertexData) {
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