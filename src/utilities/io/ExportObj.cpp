#include "ExportObj.h"
#include <fstream>
#include <iostream>
#include "../../ecs/components/Mesh.h"

void ExportObj::exportObj(Mesh& mesh, const std::string& path, const std::string& filename) {
    std::ofstream objFile(std::string(path + std::string("/") + filename));
    if (!objFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << path + std::string("/") + filename << std::endl;
        return;
    }

    // Write vertex data
    for (const auto& vertex : mesh.m_vertexData) {
        objFile << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << "\n";
    }
    for (const auto& vertex : mesh.m_vertexData) {
        objFile << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << "\n";
    }
    for (const auto& vertex : mesh.m_vertexData) {
        objFile << "vt " << vertex.uv.x << " " << vertex.uv.y << "\n";
    }
        
    // Write faces (assuming triangles)
    std::vector<uint32_t> indices = mesh.getIndexData();
    for (uint32_t i = 0; i < indices.size() - 3; i += 3) {
        objFile << "f ";
        for (int j = 0; j < 3; j++) {
            objFile
            << indices[i + j] + 1 << "/"
            << indices[i + j] + 1 << "/"
            << indices[i + j] + 1 << " ";
        }
        
        objFile << "\n";
    }

    objFile.close();
    std::cout << "Mesh saved to " << path + std::string("/") + filename << std::endl;
}