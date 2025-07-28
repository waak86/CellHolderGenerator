#include "cell_layout.h"
#include <cmath>
#include <algorithm>

app::CellLayout::FitResult app::CellLayout::fitRect(
    float width,
    float height,
    float cell_dia,
    float spacing,
    float wall_thickness,
    int series,
    int parallel,
    bool honeycomb
) {
    float pitch = cell_dia + spacing;
    float vSpacing = honeycomb ? pitch * 0.86602540378f : pitch;
    float reqW = series * pitch + 2 * wall_thickness + (honeycomb ? 0.5f * pitch : 0.0f);
    float reqH = parallel * vSpacing + 2 * wall_thickness;
    bool fits = (reqW <= width && reqH <= height);
    int maxS = series;
    int maxP = parallel;
    if(!fits) {
        if(reqW > width) {
            maxS = std::max(0, int((width - 2 * wall_thickness - (honeycomb ? 0.5f * pitch : 0.0f)) / pitch));
        }
        if(reqH > height) {
            maxP = std::max(0, int((height - 2 * wall_thickness) / vSpacing));
        }
    }
    return { fits, maxS, maxP, reqW, reqH,
             std::max(0.0f, reqW - width),
             std::max(0.0f, reqH - height) };
}

std::vector<std::vector<Vec2>> app::CellLayout::rectangleFixed(
    float width,
    float height,
    float cell_dia,
    float spacing,
    float wall_thickness,
    int series,
    int parallel,
    int segs,
    bool honeycomb
) {
    float pitch = cell_dia + spacing;
    float vSpacing = honeycomb ? pitch * 0.86602540378f : pitch;
    float R = cell_dia * 0.5f;

    std::vector<std::vector<Vec2>> rings;
    rings.reserve(1 + series * parallel);
    rings.push_back({ {0.f, 0.f}, {width, 0.f}, {width, height}, {0.f, height} });

    for(int row = 0; row < parallel; ++row) {
        for(int col = 0; col < series; ++col) {
            float x = wall_thickness
                + (col + 0.5f) * pitch
                + (honeycomb && (row % 2) ? 0.5f * pitch : 0.0f);
            float y = wall_thickness + (row + 0.5f) * vSpacing;
            std::vector<Vec2> hole;
            hole.reserve(segs);
            for(int i = 0; i < segs; ++i) {
                float ang = 2.f * M_PI * i / segs;
                hole.push_back({ x + R * std::cos(ang),
                                 y + R * std::sin(ang) });
            }
            std::reverse(hole.begin(), hole.end());
            rings.push_back(std::move(hole));
        }
    }

    return rings;
}