#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <cmath>
#include "math_utils.h"

using namespace std;

// ─── parse_vertex ───────────────────────────────────────────────────────────
// "Generates a Vertex from the unprocessed string"
// Input line example: "v 11  -23  20"
inline Vertex parse_vertex(const string& line) {
    istringstream iss(line);
    string prefix;
    float x, y, z;
    iss >> prefix >> x >> y >> z;
    return Vertex(x, y, z);
}

// ─── parse_face ─────────────────────────────────────────────────────────────
// "Parses a face string into vertex indices (0-indexed). Potentially >3 dims"
// Input line example: "f 1 2 3"
inline vector<int> parse_face(const string& line) {
    istringstream iss(line);
    string prefix;
    iss >> prefix;
    vector<int> indices;
    int idx;
    while (iss >> idx) {
        indices.push_back(idx - 1);  // OBJ is 1-indexed → convert to 0-indexed
    }
    return indices;
}

// ─── parse_obj ──────────────────────────────────────────────────────────────
inline void parse_obj(const string& filename,
                      vector<vector<int>>& faces,
                      vector<Vertex>& vertices) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == 'v' && (line.size() < 2 || line[1] == ' ')) {
            vertices.push_back(parse_vertex(line));
        } else if (line[0] == 'f') {
            faces.push_back(parse_face(line));
        }
    }
}

// ─── center_vertices ────────────────────────────────────────────────────────
// "Corrects any offsets in the vertices for better printing."
inline float center_vertices(vector<Vertex>& vertices, float base_offset) {
    float x_min =  numeric_limits<float>::max();
    float y_min =  numeric_limits<float>::max();
    float z_min =  numeric_limits<float>::max();
    float x_max = -numeric_limits<float>::max();
    float y_max = -numeric_limits<float>::max();
    float z_max = -numeric_limits<float>::max();

    for (const auto& v : vertices) {
        if (v.x < x_min) x_min = v.x;
        if (v.y < y_min) y_min = v.y;
        if (v.z < z_min) z_min = v.z;
        if (v.x > x_max) x_max = v.x;
        if (v.y > y_max) y_max = v.y;
        if (v.z > z_max) z_max = v.z;
    }

    if (z_min != 0) {
        cerr << "WARNING: Base height is not zero. Compensating." << endl;
        for (auto& v : vertices) v.z -= z_min;
        z_max -= z_min;
    }

    for (auto& v : vertices) v.z += base_offset;
    z_max += base_offset;

    float x_offset = (x_max + x_min) / 2.0f;
    if (x_offset != 0) {
        cerr << "WARNING: X-axis is not centered. Centering." << endl;
        for (auto& v : vertices) v.x -= x_offset;
    }

    float y_offset = (y_max + y_min) / 2.0f;
    if (y_offset != 0) {
        cerr << "WARNING: Y-axis is not centered. Centering." << endl;
        for (auto& v : vertices) v.y -= y_offset;
    }

    // Recompute after centering
    x_min = y_min = z_min =  numeric_limits<float>::max();
    x_max = y_max = z_max = -numeric_limits<float>::max();
    for (const auto& v : vertices) {
        if (v.x < x_min) x_min = v.x;
        if (v.y < y_min) y_min = v.y;
        if (v.z < z_min) z_min = v.z;
        if (v.x > x_max) x_max = v.x;
        if (v.y > y_max) y_max = v.y;
        if (v.z > z_max) z_max = v.z;
    }

    cerr << "INFO: Dimensions: X:(" << x_min << "," << x_max
         << ") Y:(" << y_min << "," << y_max
         << ") Z:(" << z_min << "," << z_max << ")" << endl;

    return z_max;
}
