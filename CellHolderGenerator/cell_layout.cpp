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
        return { fits, maxS, maxP, reqW, reqH,
                 std::max(0.f, reqW - width), std::max(0.f, reqH - height), 0.f };
    }

    float pitch_y = D + S;
    float baseW = 2 * (t + S) + (series - 1) * pitch + D;
    float baseH = 2 * (t + S) + (parallel - 1) * pitch_y + D;

    if(baseH > height) {
        int maxS = std::max(0, int((width - 2 * (t + S) - D) / pitch) + 1);
        int maxP = std::max(0, int((height - 2 * (t + S) - D) / pitch_y) + 1);
        return { false, maxS, maxP, baseW, baseH,
                 std::max(0.f, baseW - width), std::max(0.f, baseH - height), 0.f };
    }

    float slackW = width - baseW;
    if(slackW < 0.f) {
        int maxS = std::max(0, int((width - 2 * (t + S) - D) / pitch) + 1);
        int maxP = std::max(0, int((height - 2 * (t + S) - D) / pitch_y) + 1);
        return { false, maxS, maxP, baseW, baseH,
                 -slackW, 0.f, 0.f };
    }

    float maxOffset = pitch;
    float offset = std::min(slackW, maxOffset);
    float angle = std::acos(std::clamp(offset / pitch, 0.f, 1.f));
    float reqW = baseW + offset;
    float reqH = baseH;

    return { true, series, parallel, reqW, reqH, 0.f, 0.f, angle };
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
    float R = 0.5f * D;
    float W = width;

    float pitch_x = D + S;
    float pitch_y = honeycomb ? pitch_x * std::sin(angle) : pitch_x;
    float offset_x = honeycomb ? pitch_x * std::cos(angle) : 0.f;

    float H = (parallel - 1) * pitch_y + D + 2 * (t + S);

    float minXc = t + S + R;
    float maxXc = W - (t + S) - R;
    float minYc = t + S + R;
    float maxYc = H - (t + S) - R;

    if(honeycomb) {
        float even_last_center = minXc + (series - 1) * pitch_x;
        float max_offset_right = maxXc - even_last_center;
        if(offset_x > max_offset_right) offset_x = max_offset_right;
    }

    std::vector<std::vector<Vec2>> rings;
    rings.reserve(1 + series * parallel);
    rings.push_back({ {t, t}, {W - t, t}, {W - t, H - t}, {t, H - t} });

    for(int row = 0; row < parallel; ++row) {
        float cy = minYc + row * pitch_y;
        float rowOffset = (honeycomb && (row % 2)) ? offset_x : 0.f;
        for(int col = 0; col < series; ++col) {
            float cx = minXc + col * pitch_x + rowOffset;

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