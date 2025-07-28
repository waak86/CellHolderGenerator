#pragma once
#include "vec2.h"

struct Vec3 {
    float x, y, z;
    Vec3();
    Vec3(float x, float y, float z);
    Vec3 operator+(const Vec3& o) const;
    Vec3 operator-(const Vec3& o) const;
    Vec3 cross(const Vec3& o) const;
    Vec3 normalize() const;
};