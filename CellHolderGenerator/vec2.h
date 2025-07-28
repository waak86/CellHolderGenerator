#pragma once

struct Vec2 {
    float x, y;
    Vec2();
    Vec2(float x, float y);
    Vec2 operator+(const Vec2& o) const;
    Vec2 operator-(const Vec2& o) const;
};