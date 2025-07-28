#include <cmath>
#include <vector>
#include <algorithm>
#include "application.h"
#include "mesh.h"
#include "stl_exporter.h"
#include "triangulator.h"
#include "cell_layout.h"

void Application::run() {
    float width = 250;
    float height = 118;
    float cell_dia = 22.0f;
    float wall_thickness = 1.5f;
    float wall_height = 5.0f;
    float spacing = 0.4f;
    int   series = 10;
    int   parallel = 6;
    int   segs = 64;
    bool  honeycomb = true;

    auto fit = app::CellLayout::fitRect(
        width, height,
        cell_dia, spacing,
        wall_thickness,
        series, parallel,
        honeycomb
    );

    if(!fit.fits) {
        std::printf(
            "%ds%dp does not fit: need +%.2f width, +%.2f height.\n"
            "Max: %ds%dp\n",
            series, parallel,
            fit.deltaWidth, fit.deltaHeight,
            fit.maxSeries, fit.maxParallel
        );
        return;
    }

    float W = std::min(width, fit.reqWidth);
    float H = std::min(height, fit.reqHeight);

    auto rings = app::CellLayout::rectangleFixed(
        W, H,
        cell_dia, spacing,
        wall_thickness,
        series, parallel,
        segs, honeycomb
    );

    auto I = geometry::triangulate(rings);
    std::vector<Vec2> V;
    V.reserve(rings.size() * segs + 4);
    for(auto& r : rings)
        for(auto& v : r)
            V.push_back(v);

    Mesh m;
    for(auto& v : V)
        m.vertices.push_back({ v.x, v.y, 0.f });
    size_t N = m.vertices.size();
    for(size_t i = 0; i < N; ++i) {
        auto& v = m.vertices[i];
        m.vertices.push_back({ v.x, v.y, wall_height });
    }

    for(size_t i = 0; i + 2 < I.size(); i += 3) {
        m.faces.push_back({ (int)I[i + 2], (int)I[i + 1], (int)I[i + 0] });
        m.faces.push_back({ (int)(I[i + 0] + N),
                            (int)(I[i + 1] + N),
                            (int)(I[i + 2] + N) });
    }

    int o0 = 0, o1 = 1, o2 = 2, o3 = 3;
    int t0 = o0 + N, t1 = o1 + N, t2 = o2 + N, t3 = o3 + N;
    m.faces.push_back({ o0,o1,t1 }); m.faces.push_back({ o0,t1,t0 });
    m.faces.push_back({ o1,o2,t2 }); m.faces.push_back({ o1,t2,t1 });
    m.faces.push_back({ o2,o3,t3 }); m.faces.push_back({ o2,t3,t2 });
    m.faces.push_back({ o3,o0,t0 }); m.faces.push_back({ o3,t0,t3 });

    size_t base = 4;
    for(int h = 0; h < series * parallel; ++h) {
        size_t s = base + h * segs;
        for(int i = 0; i < segs; ++i) {
            int i0 = (int)(s + i);
            int i1 = (int)(s + (i + 1) % segs);
            m.faces.push_back({ i0, i1, i1 + (int)N });
            m.faces.push_back({ i0, i1 + (int)N, i0 + (int)N });
        }
    }

    STLExporter::export_ascii(m, "cellholder.stl");
}