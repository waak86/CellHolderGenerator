#include <cstdio>
#include <algorithm>
#include "application.h"
#include "mesh.h"
#include "stl_exporter.h"
#include "triangulator.h"
#include "cell_layout.h"
#include "dxf_exporter.h"

void Application::run() {
    // ----------------------------------------------------------
    // ---------------------- STL Exporter ----------------------
    // ----------------------------------------------------------

    float userWidth = 460.0f;
    float userHeight = 140.0f;
    float cell_dia = 21.4f;
    float spacing = 0.5f;
    float wall_thickness = 0.5f;
    float wall_height = 10.0f;
    int   series = 20;
    int   parallel = 6;
    bool  honeycomb = true;
    float chord_tol_mm = 0.005f;
    bool  rounded_corners = true;
    float corner_radius = 5.0f;

    auto fit = app::CellLayout::fitRect(
        userWidth, userHeight, cell_dia, spacing,
        wall_thickness, series, parallel, honeycomb
    );
    if(!fit.fits) {
        std::printf("%ds%dp won't fit: need +%.2fmm width, +%.2fmm height. Max: %ds%dp\n",
            series, parallel, fit.deltaWidth, fit.deltaHeight,
            fit.maxSeries, fit.maxParallel);
        return;
    }

    float W = std::min(userWidth, fit.reqWidth);
    float H = std::min(userHeight, fit.reqHeight);

    auto rings = app::CellLayout::rectangleFixed(
        W, H, cell_dia, spacing, wall_thickness,
        series, parallel, chord_tol_mm, honeycomb,
        rounded_corners, corner_radius
    );

    std::vector<size_t> ring_start, ring_size;
    std::vector<Vec2> V;
    ring_start.reserve(rings.size());
    ring_size.reserve(rings.size());
    for(const auto& r : rings) {
        ring_start.push_back(V.size());
        ring_size.push_back(r.size());
        V.insert(V.end(), r.begin(), r.end());
    }

    auto I = geometry::triangulate(rings);

    Mesh m;
    m.vertices.reserve(V.size() * 2);
    for(auto& v : V) m.vertices.push_back({ v.x, v.y, 0.f });
    size_t N = m.vertices.size();
    for(size_t i = 0; i < N; ++i) {
        auto& v = m.vertices[i];
        m.vertices.push_back({ v.x, v.y, wall_height });
    }

    for(size_t i = 0; i + 2 < I.size(); i += 3) {
        m.faces.push_back({ int(I[i + 2]), int(I[i + 1]), int(I[i]) });
        m.faces.push_back({ int(I[i] + N), int(I[i + 1] + N), int(I[i + 2] + N) });
    }

    for(size_t h = 0; h < rings.size(); ++h) {
        size_t s = ring_start[h];
        size_t n = ring_size[h];
        for(size_t i = 0; i < n; ++i) {
            int i0 = int(s + i);
            int i1 = int(s + (i + 1) % n);
            m.faces.push_back({ i0, i1, i1 + int(N) });
            m.faces.push_back({ i0, i1 + int(N), i0 + int(N) });
        }
    }

    STLExporter::export_ascii(m, "cellholder.stl");

    // --------------------------------------------------------
	// ---------------------- DXF Export ----------------------
	// --------------------------------------------------------

    float plate_side_clearance = 6.0f; // side clearance
    float end_margin = 6.0f; // top clearance
    float weld_diameter = 6.0f;
    float gap_mm = 10.f;

    auto drawing = dxf::busbars_series_groups(
        rings, series, parallel, honeycomb,
        plate_side_clearance, end_margin, weld_diameter, gap_mm
    );

    bool dxf_show_cells = true;
    float dxf_cell_diameter = cell_dia;

    auto centroid2d = [](const std::vector<Vec2>& p) {
        double cx = 0.0, cy = 0.0; size_t n = p.size();
        for(const auto& v : p) { cx += v.x; cy += v.y; }
        return Vec2{ float(cx / double(n)), float(cy / double(n)) };
        };

    if(dxf_show_cells) {
        float r = 0.5f * dxf_cell_diameter;
        for(size_t i = 1; i < rings.size(); ++i) {
            Vec2 c = centroid2d(rings[i]);
            drawing.circles.push_back({ c.x, c.y, r, "CELLS" });
        }
    }

    dxf::save(drawing, "busbars.dxf");

}