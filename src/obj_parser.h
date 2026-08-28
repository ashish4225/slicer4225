#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>
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

// ─── parse_face_index ───────────────────────────────────────────────────────
// Parses a single OBJ face token like "1", "1/2", "1/2/3", or "1//3"
// and returns the vertex index (0-indexed).
inline int parse_face_index(const string& token) {
    // Extract vertex index (everything before the first '/')
    string vertex_part;
    size_t slash_pos = token.find('/');
    if (slash_pos != string::npos) {
        vertex_part = token.substr(0, slash_pos);
    } else {
        vertex_part = token;
    }
    return stoi(vertex_part) - 1;  // OBJ is 1-indexed → convert to 0-indexed
}

// ─── parse_face ─────────────────────────────────────────────────────────────
// "Parses a face string into vertex indices (0-indexed). Potentially >3 dims"
// Supports all OBJ face formats:
//   "f 1 2 3"           (vertex only)
//   "f 1/2 3/4 5/6"     (vertex/texcoord)
//   "f 1/2/3 4/5/6"     (vertex/texcoord/normal)
//   "f 1//3 4//6 7//9"  (vertex//normal — Onshape, Blender, etc.)
inline vector<int> parse_face(const string& line) {
    istringstream iss(line);
    string prefix;
    iss >> prefix;
    vector<int> indices;
    string token;
    while (iss >> token) {
        try {
            indices.push_back(parse_face_index(token));
        } catch (...) {
            // Skip tokens that can't be parsed (shouldn't happen with valid OBJ)
        }
    }
    return indices;
}

// ─── detect_unit_scale ──────────────────────────────────────────────────────
// Scans OBJ comments for unit hints (e.g. "Units = meters") and returns a
// scale factor to convert to millimeters.  Returns 1.0 if no hint found.
inline float detect_unit_scale(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return 1.0f;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        // Only scan comment lines
        if (line[0] != '#') {
            // Stop scanning once we hit geometry data
            if (line[0] == 'v' || line[0] == 'f') break;
            continue;
        }
        // Convert to lowercase for matching
        string lower = line;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("meters") != string::npos ||
            lower.find("unit") != string::npos) {
            // Check for "meters" (not "millimeters" or "centimeters")
            if (lower.find("millimeters") != string::npos ||
                lower.find("millimetres") != string::npos) {
                return 1.0f;
            } else if (lower.find("centimeters") != string::npos ||
                       lower.find("centimetres") != string::npos) {
                return 10.0f;
            } else if (lower.find("meters") != string::npos ||
                       lower.find("metres") != string::npos) {
                return 1000.0f;
            } else if (lower.find("inches") != string::npos ||
                       lower.find("inch") != string::npos) {
                return 25.4f;
            }
        }
    }
    return 1.0f;
}

// ─── auto_detect_and_scale ──────────────────────────────────────────────────
// Heuristic: if the bounding box of the model is very small (< 1mm in all dims),
// it's likely in meters. If < 25mm in all dims and has comment hint, use that.
// This is a safety net for files that don't declare units in comments.
inline float auto_detect_scale(const vector<Vertex>& vertices) {
    if (vertices.empty()) return 1.0f;

    float x_min =  numeric_limits<float>::max(), x_max = -numeric_limits<float>::max();
    float y_min =  numeric_limits<float>::max(), y_max = -numeric_limits<float>::max();
    float z_min =  numeric_limits<float>::max(), z_max = -numeric_limits<float>::max();

    for (const auto& v : vertices) {
        if (v.x < x_min) x_min = v.x;
        if (v.y < y_min) y_min = v.y;
        if (v.z < z_min) z_min = v.z;
        if (v.x > x_max) x_max = v.x;
        if (v.y > y_max) y_max = v.y;
        if (v.z > z_max) z_max = v.z;
    }

    float x_span = x_max - x_min;
    float y_span = y_max - y_min;
    float z_span = z_max - z_min;
    float max_span = max({x_span, y_span, z_span});

    // If the largest dimension is < 1, likely in meters
    if (max_span > 0 && max_span < 1.0f) {
        return 1000.0f;  // meters → mm
    }
    // If the largest dimension is between 1 and 10, could be centimeters
    if (max_span >= 1.0f && max_span < 10.0f) {
        return 10.0f;  // cm → mm
    }
    return 1.0f;
}

// ─── parse_obj ──────────────────────────────────────────────────────────────
inline void parse_obj(const string& filename,
                      vector<vector<int>>& faces,
                      vector<Vertex>& vertices) {
    // First pass: detect unit scale from comments
    float unit_scale = detect_unit_scale(filename);

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

    // If no unit hint was found in comments, use heuristic detection
    if (unit_scale == 1.0f) {
        unit_scale = auto_detect_scale(vertices);
    }

    // Apply unit scaling if needed
    if (unit_scale != 1.0f) {
        cerr << "INFO: Detected unit scale factor: " << unit_scale
             << "x (converting to mm)" << endl;
        for (auto& v : vertices) {
            v.x *= unit_scale;
            v.y *= unit_scale;
            v.z *= unit_scale;
        }
    }

    // Triangulate any faces with more than 3 vertices (fan triangulation)
    vector<vector<int>> triangulated_faces;
    for (const auto& face : faces) {
        if (face.size() <= 3) {
            triangulated_faces.push_back(face);
        } else {
            // Fan triangulation: pick first vertex as pivot
            for (size_t i = 1; i + 1 < face.size(); ++i) {
                triangulated_faces.push_back({face[0], face[static_cast<int>(i)], face[static_cast<int>(i + 1)]});
            }
        }
    }
    faces = triangulated_faces;
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
