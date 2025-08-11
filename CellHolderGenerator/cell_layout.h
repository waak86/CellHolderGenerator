#pragma once
#include <vector>
#include "vec2.h"

namespace app {
    struct CellLayout {
        struct FitResult {
            bool  fits;
            int   maxSeries;
            int   maxParallel;
            float reqWidth;
            float reqHeight;
            float deltaWidth;
            float deltaHeight;
        };

        static FitResult fitRect(
            float width,
            float height,
            float cell_dia,
            float spacing,
            float wall_thickness,
            int series,
            int parallel,
            bool honeycomb
        );

        static std::vector<std::vector<Vec2>> rectangleFixed(
            float width,
            float height,
            float cell_dia,
            float spacing,
            float wall_thickness,
            int series,
            int parallel,
            float chord_tol_mm,
            bool honeycomb,
            bool rounded_corners = false,
            float corner_radius = 5.0f
        );
    };
}