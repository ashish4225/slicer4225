#pragma once

#include <cmath>
#include <stdexcept>
#include <array>

using namespace std;

// ─── Vertex ─────────────────────────────────────────────────────────────────
struct Vertex {
    float x, y, z;

    Vertex() : x(0), y(0), z(0) {}
    Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float  operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
    float& operator[](int i)       { return i == 0 ? x : (i == 1 ? y : z); }

    bool operator==(const Vertex& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
    bool operator!=(const Vertex& o) const { return !(*this == o); }
    bool operator<(const Vertex& o) const {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }
};

// ─── Axis enum (mirrors Python IntEnum) ─────────────────────────────────────
enum class Axis { X = 0, Y = 1, Z = 2 };

// ─── get_intersection ───────────────────────────────────────────────────────
// "Calculate the intersection of the line segment c1→c2 given a fixed x, y, or z coord."
inline Vertex get_intersection_x(const Vertex& c1, const Vertex& c2, float xval) {
    float y = (xval - c1.x) * (c2.y - c1.y) / (c2.x - c1.x) + c1.y;
    float z = (xval - c1.x) * (c2.z - c1.z) / (c2.x - c1.x) + c1.z;
    return Vertex(xval, y, z);
}

inline Vertex get_intersection_y(const Vertex& c1, const Vertex& c2, float yval) {
    float x = (yval - c1.y) * (c2.x - c1.x) / (c2.y - c1.y) + c1.x;
    float z = (yval - c1.y) * (c2.z - c1.z) / (c2.y - c1.y) + c1.z;
    return Vertex(x, yval, z);
}

inline Vertex get_intersection_z(const Vertex& c1, const Vertex& c2, float zval) {
    float x = (zval - c1.z) * (c2.x - c1.x) / (c2.z - c1.z) + c1.x;
    float y = (zval - c1.z) * (c2.y - c1.y) / (c2.z - c1.z) + c1.y;
    return Vertex(x, y, zval);
}

// General version that dispatches on axis
inline Vertex get_intersection(const Vertex& c1, const Vertex& c2, Axis axis, float val) {
    switch (axis) {
        case Axis::X: return get_intersection_x(c1, c2, val);
        case Axis::Y: return get_intersection_y(c1, c2, val);
        case Axis::Z: return get_intersection_z(c1, c2, val);
    }
    throw runtime_error("Invalid axis");
}

// ─── distance_between ───────────────────────────────────────────────────────
inline float distance_between(const Vertex& c1, const Vertex& c2) {
    float dx = c1.x - c2.x;
    float dy = c1.y - c2.y;
    float dz = c1.z - c2.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}
