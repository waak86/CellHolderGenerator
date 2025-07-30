#include "cell_layout.h"
#include <cmath>
#include <algorithm>

using app::CellLayout;
using FitResult = CellLayout::FitResult;

FitResult CellLayout::fitRect(
    float width,
    float height,
    float cell_dia,
    float spacing,
    float wall_thickness,
    int series,
    int parallel,
    bool honeycomb
) {
    float D = cell_dia;
    float S = spacing;
    float t = wall_thickness;
    float pitch = D + S;

    if(!honeycomb) {
        float reqW = series * pitch + 2 * t;
        float reqH = parallel * pitch + 2 * t;
        bool fits = (reqW <= width && reqH <= height);
        int maxS = fits ? series : std::max(0, int((width - 2 * t) / pitch));
        int maxP = fits ? parallel : std::max(0, int((height - 2 * t) / pitch));
        return { fits, maxS, maxP,
                 reqW, reqH,
                 std::max(0.f, reqW - width),
                 std::max(0.f, reqH - height),
                 0.f };
    }

    // honeycomb case
    float cw = (width - 2 * t - series * pitch) / pitch;
    cw = std::clamp(cw, 0.f, 1.f);
    float angleW = std::acos(cw);

    float sh = (height - 2 * t) / (parallel * pitch);
    sh = std::clamp(sh, 0.f, 1.f);
    float angleH = std::asin(sh);

    if(angleW <= angleH) {
        float reqW = series * pitch + 2 * t + pitch * std::cos(angleW);
        float reqH = parallel * pitch * std::sin(angleW) + 2 * t;
        return { true, series, parallel,
                 reqW, reqH,
                 0.f, 0.f,
                 angleW };
    }

    // cannot fit honeycomb at any angle
    // fall back to maximum grid values
    int maxS = std::max(0, int((width - 2 * t) / pitch));
    int maxP = std::max(0, int((height - 2 * t) / pitch));
    float reqW = series * pitch + 2 * t + pitch * std::cos(angleW);
    float reqH = parallel * pitch * std::sin(angleH) + 2 * t;
    return { false, maxS, maxP,
             reqW, reqH,
             std::max(0.f, reqW - width),
             std::max(0.f, reqH - height),
             0.f };
}

std::vector<std::vector<Vec2>> CellLayout::rectangleFixed(
    float width,
    float height,
    float cell_dia,
    float spacing,
    float wall_thickness,
    int series,
    int parallel,
    int segs,
    float angle,
    bool honeycomb
) {
    float D = cell_dia;
    float S = spacing;
    float t = wall_thickness;
    float pitch = D + S;
    float R = D * 0.5f;

    float offset = honeycomb ? pitch * std::cos(angle) : 0.f;
    float vStep = honeycomb ? pitch * std::sin(angle) : pitch;

    float W = series * pitch + 2 * t + offset;
    float H = honeycomb
        ? parallel * vStep + 2 * t
        : parallel * pitch + 2 * t;

    std::vector<std::vector<Vec2>> rings;
    rings.reserve(1 + series * parallel);
    rings.push_back({ {t, t}, {W - t, t}, {W - t, H - t}, {t, H - t} });

    for(int row = 0; row < parallel; ++row) {
        for(int col = 0; col < series; ++col) {
            float cx = t + R + col * pitch + (honeycomb && (row % 2) ? offset : 0);
            float cy = t + R + row * vStep;
            std::vector<Vec2> hole;
            hole.reserve(segs);
            for(int i = 0; i < segs; ++i) {
                float a = 2.f * M_PI * i / segs;
                hole.push_back({ cx + R * std::cos(a), cy + R * std::sin(a) });
            }
            std::reverse(hole.begin(), hole.end());
            rings.push_back(std::move(hole));
        }
    }
    return rings;
}