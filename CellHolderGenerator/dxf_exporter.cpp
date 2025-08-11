#include "dxf_exporter.h"
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace dxf;

static inline Vec2 centroid(const std::vector<Vec2>& p) {
    double cx = 0.0, cy = 0.0; size_t n = p.size();
    for(const auto& v : p) { cx += v.x; cy += v.y; }
    return { float(cx / double(n)), float(cy / double(n)) };
}

Drawing dxf::busbars_series_groups(
    const std::vector<std::vector<Vec2>>& rings,
    int series,
    int parallel,
    bool honeycomb,
    float plate_side_clearance,
    float end_margin,
    float weld_diameter,
    float gap_mm
) {
    std::vector<Vec2> C; C.reserve(size_t(series) * size_t(parallel));
    for(int r = 0; r < parallel; ++r)
        for(int c = 0; c < series; ++c)
            C.push_back(centroid(rings[1 + r * series + c]));

    Drawing d;
    float r_weld = 0.5f * weld_diameter;
    float halfGap = 0.5f * std::max(0.f, gap_mm);

    if(!honeycomb) {
        std::vector<float> meanX(series), midX(series - 1);
        float minx = 1e30f, maxx = -1e30f, miny = 1e30f, maxy = -1e30f;
        for(int c = 0; c < series; ++c) {
            double sx = 0.0;
            for(int r = 0; r < parallel; ++r) {
                const Vec2& P = C[r * series + c];
                sx += P.x;
                minx = std::min(minx, P.x);
                maxx = std::max(maxx, P.x);
                miny = std::min(miny, P.y);
                maxy = std::max(maxy, P.y);
            }
            meanX[c] = float(sx / std::max(1, parallel));
        }
        for(int c = 1; c < series; ++c) midX[c - 1] = 0.5f * (meanX[c - 1] + meanX[c]);

        float outerL = minx - plate_side_clearance;
        float outerR = maxx + plate_side_clearance;

        std::vector<std::vector<int>> groups;
        if(series <= 1) groups.push_back({ 0 });
        else {
            groups.push_back({ 0 });
            for(int c = 1; c + 1 <= series - 1; c += 2) groups.push_back({ c,c + 1 });
            groups.push_back({ series - 1 });
        }

        for(const auto& g : groups) {
            float xL, xR;
            int kind = (g.size() == 1 ? (g[0] == 0 ? -1 : 1) : 0);
            if(kind == -1) {
                xL = outerL;
                xR = midX[0] - halfGap;
            }
            else if(kind == 1) {
                xL = midX[series - 2] + halfGap;
                xR = outerR;
            }
            else {
                int c0 = g[0], c1 = g[1];
                xL = midX[c0 - 1] + halfGap;
                xR = midX[c1] - halfGap;
            }

            float y0 = miny - end_margin;
            float y1 = maxy + end_margin;

            d.polylines.push_back({ {{xL,y0},{xR,y0},{xR,y1},{xL,y1}}, true, (kind == -1 ? "B-" : (kind == 1 ? "B+" : "BUSBAR")) });

            //for(int r = 0; r < parallel; ++r)
            //    for(int col : g)
            //        d.circles.push_back({ C[r * series + col].x, C[r * series + col].y, r_weld, (kind == -1 ? "B-" : (kind == 1 ? "B+" : "BUSBAR")) });
        }

        return d;
    }

    std::vector<float> rowY(parallel);
    float minx = 1e30f, maxx = -1e30f;
    for(int r = 0; r < parallel; ++r) {
        rowY[r] = C[r * series + 0].y;
        for(int c = 0; c < series; ++c) {
            float x = C[r * series + c].x;
            minx = std::min(minx, x);
            maxx = std::max(maxx, x);
        }
    }
    float outerL = minx - plate_side_clearance;
    float outerR = maxx + plate_side_clearance;

    std::vector<std::vector<int>> groups;
    if(series <= 1) groups.push_back({ 0 });
    else {
        groups.push_back({ 0 });
        for(int c = 1; c + 1 <= series - 1; c += 2) groups.push_back({ c,c + 1 });
        groups.push_back({ series - 1 });
    }

    auto midL_at = [&](int r, int col)->float {
        if(col <= 0) return outerL;
        float xl = C[r * series + (col - 1)].x;
        float xr = C[r * series + col].x;
        return 0.5f * (xl + xr);
        };
    auto midR_at = [&](int r, int col)->float {
        if(col >= series - 1) return outerR;
        float xl = C[r * series + col].x;
        float xr = C[r * series + (col + 1)].x;
        return 0.5f * (xl + xr);
        };

    for(const auto& g : groups) {
        int kind = (g.size() == 1 ? (g[0] == 0 ? -1 : 1) : 0);

        std::vector<float> L(parallel), R(parallel);
        for(int r = 0; r < parallel; ++r) {
            if(kind == -1) {
                L[r] = outerL;
                R[r] = midR_at(r, 0) - halfGap;
            }
            else if(kind == 1) {
                L[r] = midL_at(r, series - 1) + halfGap;
                R[r] = outerR;
            }
            else {
                int c0 = g[0], c1 = g[1];
                L[r] = midL_at(r, c0) + halfGap;
                R[r] = midR_at(r, c1) - halfGap;
            }
            //for(int col : g)
            //    d.circles.push_back({ C[r * series + col].x, C[r * series + col].y, r_weld, (kind == -1 ? "B-" : (kind == 1 ? "B+" : "BUSBAR")) });
        }

        float yTop = rowY.front() - end_margin;
        float yBot = rowY.back() + end_margin;

        std::vector<Vec2> leftPath;  leftPath.reserve(parallel + 2);
        std::vector<Vec2> rightPath; rightPath.reserve(parallel + 2);

        leftPath.push_back({ (kind == -1 ? outerL : L[0]), yTop });
        for(int r = 0; r < parallel; ++r) leftPath.push_back({ (kind == -1 ? outerL : L[r]), rowY[r] });
        leftPath.push_back({ (kind == -1 ? outerL : L.back()), yBot });

        rightPath.push_back({ (kind == 1 ? outerR : R.back()), yBot });
        for(int r = parallel - 1; r >= 0; --r) rightPath.push_back({ (kind == 1 ? outerR : R[r]), rowY[r] });
        rightPath.push_back({ (kind == 1 ? outerR : R[0]), yTop });

        std::vector<Vec2> poly;
        poly.reserve(leftPath.size() + rightPath.size());
        poly.insert(poly.end(), leftPath.begin(), leftPath.end());
        poly.insert(poly.end(), rightPath.begin(), rightPath.end());

        d.polylines.push_back({ poly, true, (kind == -1 ? "B-" : (kind == 1 ? "B+" : "BUSBAR")) });
    }

    return d;
}

void dxf::save(const Drawing& d, const char* filename) {
    std::ofstream out(filename, std::ios::binary);
    out << "0\nSECTION\n2\nENTITIES\n";
    for(const auto& pl : d.polylines) {
        out << "0\nLWPOLYLINE\n8\n" << (pl.layer.empty() ? "0" : pl.layer)
            << "\n90\n" << int(pl.pts.size()) << "\n70\n" << (pl.closed ? 1 : 0) << "\n";
        for(const auto& p : pl.pts) { out << "10\n" << p.x << "\n20\n" << p.y << "\n"; }
    }
    for(const auto& c : d.circles) {
        out << "0\nCIRCLE\n8\n" << (c.layer.empty() ? "0" : c.layer)
            << "\n10\n" << c.cx << "\n20\n" << c.cy << "\n30\n0\n40\n" << c.r << "\n";
    }
    out << "0\nENDSEC\n0\nEOF\n";
}