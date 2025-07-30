#include <cstdio>
#include <algorithm>
#include "application.h"
#include "mesh.h"
#include "stl_exporter.h"
#include "triangulator.h"
#include "cell_layout.h"

void Application::run() {
    float userWidth = 445.0f;
    float userHeight = 140.0f;
    float cell_dia = 21.4f;
    float spacing = 0.4f;
    float wall_thickness = 0.4f;
    float wall_height = 5.0f;
    int   series = 20;
    int   parallel = 6;
    int   segs = 64;
    bool  honeycomb = true;

    auto fit = app::CellLayout::fitRect(
        userWidth, userHeight,
        cell_dia, spacing,
        wall_thickness,
        series, parallel,
        honeycomb
    );

    if(!fit.fits) {
        std::printf(
            "%ds%dp won't fit: need +%.2fmm width, +%.2fmm height.\nMax: %ds%dp\n",
            series, parallel,
            fit.deltaWidth, fit.deltaHeight,
            fit.maxSeries, fit.maxParallel
        );
        return;
    }

    float W = std::min(userWidth, fit.reqWidth);
    float H = std::min(userHeight, fit.reqHeight);

    auto rings = app::CellLayout::rectangleFixed(
        W, H,
        cell_dia, spacing,
        wall_thickness,
        series, parallel,
        segs, fit.angle, honeycomb
    );

    auto I = geometry::triangulate(rings);
    std::vector<Vec2> V;
    for(auto& r : rings)
        for(auto& v : r)
            V.push_back(v);

    Mesh m;
    for(auto& v : V) m.vertices.push_back({ v.x,v.y,0.f });
    size_t N = m.vertices.size();
    for(size_t i = 0; i < N; ++i) {
        auto& v = m.vertices[i];
        m.vertices.push_back({ v.x,v.y,wall_height });
    }

    for(size_t i = 0; i + 2 < I.size(); i += 3) {
        m.faces.push_back({ (int)I[i + 2],(int)I[i + 1],(int)I[i] });
        m.faces.push_back({ (int)(I[i] + N),(int)(I[i + 1] + N),(int)(I[i + 2] + N) });
    }

    int o0 = 0, o1 = 1, o2 = 2, o3 = 3;
    int t0 = o0 + N, t1 = o1 + N, t2 = o2 + N, t3 = o3 + N;
    m.faces.push_back({ o0,o1,t1 }); m.faces.push_back({ o0,t1,t0 });
    m.faces.push_back({ o1,o2,t2 }); m.faces.push_back({ o1,t2,t1 });
    m.faces.push_back({ o2,o3,t3 }); m.faces.push_back({ o2,t3,t2 });
    m.faces.push_back({ o3,o0,t0 }); m.faces.push_back({ o3,t0,t3 });

    size_t base = rings[0].size();
    for(int h = 0; h < series * parallel; ++h) {
        size_t s = base + h * segs;
        for(int i = 0; i < segs; ++i) {
            int i0 = s + i;
            int i1 = s + (i + 1) % segs;
            m.faces.push_back({ i0,i1,i1 + (int)N });
            m.faces.push_back({ i0,i1 + (int)N,i0 + (int)N });
        }
    }

    STLExporter::export_ascii(m, "cellholder.stl");
}