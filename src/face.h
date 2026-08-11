#pragma once

#include <vector>
#include <string>
#include <sstream>
#include "math_utils.h"

using namespace std;

// ─── Face ───────────────────────────────────────────────────────────────────
// Mirrors the Python Face class from slicer.py
struct Face {
    vector<int> vertex_indices;   // indices into the global vertex array
    int face_num;
    vector<Vertex> contour_points;

    Face() : face_num(-1) {}
    Face(const vector<int>& v, int num) : vertex_indices(v), face_num(num) {}

    void add_contour_pts(const Vertex& pt) {
        contour_points.push_back(pt);
    }

    string to_string() const {
        ostringstream oss;
        oss << "Face " << face_num << ": [";
        for (size_t i = 0; i < vertex_indices.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << vertex_indices[i];
        }
        oss << "]";
        if (!contour_points.empty()) {
            oss << " contour:[";
            for (size_t i = 0; i < contour_points.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "(" << contour_points[i].x << "," << contour_points[i].y << "," << contour_points[i].z << ")";
            }
            oss << "]";
        }
        return oss.str();
    }
};

// ─── FaceQueue ──────────────────────────────────────────────────────────────
// Mirrors the Python FaceQueue class from slicer.py
class FaceQueue {
public:
    vector<Face> q;
    vector<Face> store;

    FaceQueue() = default;

    size_t size() const { return q.size(); }

    const Face& operator[](size_t idx) const { return q[idx]; }
    Face& operator[](size_t idx) { return q[idx]; }

    // Count how many vertices of `face` also appear in the last face of q
    int get_matches(const Face& face) const {
        int count = 0;
        for (int idx : q.back().vertex_indices) {
            for (int fidx : face.vertex_indices) {
                if (idx == fidx) {
                    ++count;
                    break;
                }
            }
        }
        return count;
    }

    // Checks if the contour is closed
    bool check_closed() const {
        if (q.size() <= 2) return false;
        // Count matches between last face and first face
        int count = 0;
        for (int idx : q.back().vertex_indices) {
            for (int fidx : q[0].vertex_indices) {
                if (idx == fidx) {
                    ++count;
                    break;
                }
            }
        }
        return count >= 2;
    }

    // Returns true when the contour is complete (closed)
    bool insert(const Face& face) {
        if (q.empty()) {
            q.push_back(face);
        } else {
            int matches = get_matches(face);
            if (matches < 2) {
                // Push to store if fewer than 2 vertices match
                store.push_back(face);
            } else if (matches == 2) {
                // Append to q if there is one edge match
                q.push_back(face);
            }

            if (!store.empty()) {
                bool added_from_store = false;
                while (true) {
                    added_from_store = false;
                    for (auto it = store.begin(); it != store.end(); ++it) {
                        if (get_matches(*it) >= 2) {
                            q.push_back(*it);
                            store.erase(it);
                            added_from_store = true;
                            break;
                        }
                    }
                    if (!added_from_store) {
                        break;
                    }
                }
            }
        }
        return check_closed();
    }
};
