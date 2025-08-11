#pragma once
#include <vector>
#include <string>
#include "vec2.h"

namespace dxf {
    struct Polyline { std::vector<Vec2> pts; bool closed; std::string layer; };
    struct Circle { float cx, cy, r; std::string layer; };

    struct Drawing {
        std::vector<Polyline> polylines;
        std::vector<Circle>   circles;
    };

    Drawing busbars_series_groups(
        const std::vector<std::vector<Vec2>>& rings,
        int series,
        int parallel,
        bool honeycomb,
        float plate_side_clearance,
        float end_margin,
        float weld_diameter,
        float gap_mm
    );

    void save(const Drawing& d, const char* filename);
}