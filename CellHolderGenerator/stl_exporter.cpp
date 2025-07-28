#include <windows.h>
#include <string>
#include <fstream>
#include "stl_exporter.h"

void STLExporter::export_ascii(const Mesh& mesh, const char* filename) {
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::string exePath(buffer);
    auto pos = exePath.find_last_of("\\/");
    std::string dir = pos != std::string::npos
        ? exePath.substr(0, pos + 1)
        : "";

    std::string outPath = dir + filename;
    std::ofstream out(outPath, std::ios::out);
    out << "solid cellholder\n";

    auto V = mesh.vertices.size();
    for(const auto& f : mesh.faces) {
        if((uint32_t)f.v1 >= V || (uint32_t)f.v2 >= V || (uint32_t)f.v3 >= V)
            continue;

        auto a = mesh.vertices[f.v1];
        auto b = mesh.vertices[f.v2];
        auto c = mesh.vertices[f.v3];
        auto n = (b - a).cross(c - a).normalize();

        out << "  facet normal " << n.x << " " << n.y << " " << n.z << "\n"
            << "    outer loop\n"
            << "      vertex " << a.x << " " << a.y << " " << a.z << "\n"
            << "      vertex " << b.x << " " << b.y << " " << b.z << "\n"
            << "      vertex " << c.x << " " << c.y << " " << c.z << "\n"
            << "    endloop\n"
            << "  endfacet\n";
    }

    out << "endsolid cellholder\n";
    out.close();
}