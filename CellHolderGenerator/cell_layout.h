#pragma once
#include <vector>
#include "vec2.h"

namespace app {
    struct CellLayout {
        static std::vector<std::vector<Vec2>> rectangle(
            float cell_dia,
            float spacing,
            float wall_thickness,
            int series,
            int parallel,
            int segs,
            bool honeycomb
        );
    };
}