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
    for (const auto& vertex : mesh.getVertexData()) {
        objFile << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << "\n";
        objFile << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << "\n";
        objFile << "vt " << vertex.uv.x << " " << vertex.uv.y << "\n";
    }

    // Write faces (assuming triangles)
    uint32_t vertexIndexOffset = mesh.getVertexBufferOffset();
    for (uint32_t i = 0; i < mesh.getVertexCount(); i += 3) {
        objFile << "f ";
        for (uint32_t j = 0; j < 3; ++j) {
            objFile << i + j + 1 + vertexIndexOffset << "/" << i + j + 1 + vertexIndexOffset << "/" << i + j + 1 + vertexIndexOffset << " ";
        }
        objFile << "\n";
    }

    objFile.close();
    std::cout << "Mesh saved to " << path + std::string("/") + filename << std::endl;
}