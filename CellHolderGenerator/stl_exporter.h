#pragma once
#include "mesh.h"

class STLExporter {
public:
    static void export_ascii(const Mesh& mesh, const char* filename);
};