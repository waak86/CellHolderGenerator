#include "vec3.h"
#include <cmath>

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
Vec3 Vec3::operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
Vec3 Vec3::operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
Vec3 Vec3::cross(const Vec3& o) const { return { y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x }; }
Vec3 Vec3::normalize() const {
    float l = std::sqrt(x * x + y * y + z * z);
    return l > 0 ? Vec3{ x / l, y / l, z / l } : Vec3{};
}