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
#include "utilities/VertexDataCalculations.h"

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

    bool success = VertexDataCalculations::vertexDataCalculations(vertexData);
    if (!success) {
        std::cout << "failed to do vertex calculations" << std::endl;
    }

    std::cout << "done loading " << path << std::endl;

    return true;
}
