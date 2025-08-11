#include "cell_layout.h"
#include <cmath>
#include <algorithm>

using app::CellLayout;
using FitResult = CellLayout::FitResult;

static inline float vstep_honey(float p) { return p * 0.8660254037844386f; }

static inline float signed_area(const std::vector<Vec2>& p) {
    double a = 0.0;
    for(size_t i = 0, n = p.size(); i < n; ++i) {
        const Vec2& u = p[i];
        const Vec2& v = p[(i + 1) % n];
        a += double(u.x) * double(v.y) - double(u.y) * double(v.x);
    }
    return float(0.5 * a);
}

static inline void ensure_orientation(std::vector<Vec2>& p, bool ccw) {
    if((signed_area(p) > 0.f) != ccw)
        std::reverse(p.begin(), p.end());
}

static inline int segs_from_tol(float R, float e, int min_segs = 64, int max_segs = 4096) {
    if(R <= 0.f) return min_segs;
    double ee = std::max(1e-6, double(e));
    double th = 2.0 * std::acos(std::max(0.0, 1.0 - ee / double(R)));
    if(!std::isfinite(th) || th <= 0.0) return max_segs;
    int n = int(std::ceil((2.0 * M_PI) / th));
    n = (n + 3) & ~3;
    return std::min(std::max(n, min_segs), max_segs);
}

static inline void append_arc_ccw(std::vector<Vec2>& out, float cx, float cy, float r,
    float a0, float a1, int steps, bool include_start) {
    for(int i = 0; i <= steps; i++) {
        if(i == 0 && !include_start) continue;
        float t = a0 + (a1 - a0) * float(i) / float(steps);
        out.push_back({ cx + r * std::cos(t), cy + r * std::sin(t) });
    }
}

FitResult CellLayout::fitRect(float width, float height, float cell_dia, float spacing,
    float wall_thickness, int series, int parallel, bool honeycomb) {
    float D = cell_dia, S = spacing, t = wall_thickness, pitch = D + S;
    float off = honeycomb ? 0.5f * pitch : 0.f;
    float vstep = honeycomb ? vstep_honey(pitch) : pitch;

    float reqW = 2 * t + 2 * (S + 0.5f * D)
        + (series > 0 ? (series - 1) * pitch : 0.f) + off;
    float reqH = 2 * t + 2 * (S + 0.5f * D)
        + (parallel > 0 ? (parallel - 1) * vstep : 0.f);

    bool ok = (reqW <= width && reqH <= height);
    int ms, mp;

    if(ok) { ms = series; mp = parallel; }
    else {
        if(!honeycomb) {
            ms = std::max(0, int((width - 2 * t - 2 * (S + 0.5f * D)) / pitch) + 1);
            mp = std::max(0, int((height - 2 * t - 2 * (S + 0.5f * D)) / pitch) + 1);
        }
        else {
            ms = std::max(0, int((width - 2 * t - 2 * (S + 0.5f * D) - off) / pitch) + 1);
            mp = std::max(0, int((height - 2 * t - 2 * (S + 0.5f * D)) / vstep) + 1);
        }
        ms = std::min(ms, series);
        mp = std::min(mp, parallel);
    }

    return { ok, ms, mp, reqW, reqH,
             ok ? 0.f : std::max(0.f, reqW - width),
             ok ? 0.f : std::max(0.f, reqH - height) };
}

std::vector<std::vector<Vec2>> CellLayout::rectangleFixed(
    float width, float height, float cell_dia, float spacing, float wall_thickness,
    int series, int parallel, float chord_tol_mm, bool honeycomb,
    bool rounded_corners, float corner_radius
) {
    float D = cell_dia, S = spacing, t = wall_thickness;
    float R = 0.5f * D, pitch = D + S;
    float minXc = t + S + R, minYc = t + S + R;
    float off = honeycomb ? 0.5f * pitch : 0.f;
    float vstep = honeycomb ? vstep_honey(pitch) : pitch;

    int segs = segs_from_tol(R, std::max(1e-4f, chord_tol_mm));
    std::vector<Vec2> unit(segs);
    for(int i = 0; i < segs; ++i) {
        float a = 2.f * float(M_PI) * float(i) / float(segs);
        unit[i] = { std::cos(a), std::sin(a) };
    }

    std::vector<std::vector<Vec2>> rings;
    rings.reserve(1 + series * parallel);

    if(!rounded_corners) {
        std::vector<Vec2> outer = { {t,t}, {width - t,t}, {width - t,height - t}, {t,height - t} };
        ensure_orientation(outer, true);
        rings.push_back(std::move(outer));
    }
    else {
        float maxr = 0.5f * std::min(width, height) - t;
        float rc = std::max(0.f, std::min(corner_radius, maxr));
        int nfull = segs_from_tol(rc, std::max(1e-4f, chord_tol_mm));
        int nquad = std::max(2, nfull / 4);
        float x0 = t, y0 = t, x1 = width - t, y1 = height - t;
        std::vector<Vec2> outer;
        outer.reserve(4 * (nquad + 1));
        outer.push_back({ x0 + rc, y0 });
        outer.push_back({ x1 - rc, y0 });
        append_arc_ccw(outer, x1 - rc, y0 + rc, rc, -0.5f * float(M_PI), 0.f, nquad, false);
        outer.push_back({ x1, y1 - rc });
        append_arc_ccw(outer, x1 - rc, y1 - rc, rc, 0.f, 0.5f * float(M_PI), nquad, false);
        outer.push_back({ x0 + rc, y1 });
        append_arc_ccw(outer, x0 + rc, y1 - rc, rc, 0.5f * float(M_PI), float(M_PI), nquad, false);
        outer.push_back({ x0, y0 + rc });
        append_arc_ccw(outer, x0 + rc, y0 + rc, rc, float(M_PI), 1.5f * float(M_PI), nquad, false);
        ensure_orientation(outer, true);
        rings.push_back(std::move(outer));
    }

    for(int row = 0; row < parallel; ++row) {
        float cy = minYc + row * vstep;
        float rowOffset = (honeycomb && (row % 2)) ? off : 0.f;
        for(int col = 0; col < series; ++col) {
            float cx = minXc + col * pitch + rowOffset;
            std::vector<Vec2> hole;
            hole.reserve(segs);
            for(int i = 0; i < segs; ++i)
                hole.push_back({ cx + R * unit[i].x, cy + R * unit[i].y });
            ensure_orientation(hole, false);
            rings.push_back(std::move(hole));
        }
    }

    return rings;
}