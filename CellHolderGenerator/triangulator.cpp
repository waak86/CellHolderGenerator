#include "triangulator.h"
#include <array>

std::vector<uint32_t> geometry::triangulate(const std::vector<std::vector<Vec2>>& rings) {
    std::vector<std::vector<std::array<double, 2>>> data;
    data.reserve(rings.size());
    for(auto& ring : rings) {
        std::vector<std::array<double, 2>> r;
        r.reserve(ring.size());
        for(auto& v : ring) r.push_back({ v.x, v.y });
        data.push_back(std::move(r));
    }
    return mapbox::earcut<uint32_t>(data);
}