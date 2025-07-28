#include "vec2.h"

Vec2::Vec2() : x(0), y(0) {}
Vec2::Vec2(float x_, float y_) : x(x_), y(y_) {}
Vec2 Vec2::operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
Vec2 Vec2::operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }