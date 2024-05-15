#include "ResourceLoader.h"
#include "tiny_obj_loader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
namespace fs = std::filesystem;

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

bool ResourceLoader::loadObj(const std::string& path, std::vector<VertexData>& vertexData) {
    std::cout << "loading .obj file: " << path << std::endl;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!success) {
        std::cerr << "could not load obj file! " << path << std::endl;
    }

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
                attrib.texcoords[2 * it.texcoord_index + 1],
            };
        }
        
    }

    std::cout << "done loading " << path << std::endl;

    return true;
}