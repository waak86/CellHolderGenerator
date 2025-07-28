#pragma once
#include <vector>
#include "vec3.h"

class Face {
public:
    int v1, v2, v3;
};

class Mesh {
public:
    std::vector<Vec3> vertices;
    std::vector<Face> faces;

    void export_as_stl(const char* path) const;
};