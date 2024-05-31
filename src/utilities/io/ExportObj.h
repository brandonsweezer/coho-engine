#pragma once
#include <iostream>
#include "../../ecs/components/Mesh.h"

class ExportObj {
public:
    static void exportObj(Mesh& mesh, const std::string& path, const std::string& filename);
};