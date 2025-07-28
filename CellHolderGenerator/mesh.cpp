#include "mesh.h"
#include <fstream>

void Mesh::export_as_stl(const char* path) const {
    std::ofstream out(path);
    out << "solid cellholder\n";
    for(const auto& f : faces) {
        Vec3 a = vertices[f.v1];
        Vec3 b = vertices[f.v2];
        Vec3 c = vertices[f.v3];
        Vec3 n = (b - a).cross(c - a).normalize();

        out << "  facet normal " << n.x << " " << n.y << " " << n.z << "\n";
        out << "    outer loop\n";
        out << "      vertex " << a.x << " " << a.y << " " << a.z << "\n";
        out << "      vertex " << b.x << " " << b.y << " " << b.z << "\n";
        out << "      vertex " << c.x << " " << c.y << " " << c.z << "\n";
        out << "    endloop\n";
        out << "  endfacet\n";
    }
    out << "endsolid cellholder\n";
}