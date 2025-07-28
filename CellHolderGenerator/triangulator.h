#pragma once
#include <vector>
#include "vec2.h"
#include "earcut.h"

namespace geometry {
    std::vector<uint32_t> triangulate(const std::vector<std::vector<Vec2>>& rings);
}