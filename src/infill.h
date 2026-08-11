#pragma once

#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include "math_utils.h"
#include "face.h"
#include "gcode_writer.h"

using namespace std;

// ─── find_faces_at_index ────────────────────────────────────────────────────
// "Find all the faces that intersect `coord_val` along `index`"
// Mirrors the Python find_faces_at_index function from infill.py
inline vector<const Face*> find_faces_at_index(
        const vector<FaceQueue>& layer_qs,
        float coord_val,
        int index) {
    vector<const Face*> result;
    for (const auto& face_q : layer_qs) {
        for (size_t i = 0; i < face_q.q.size(); ++i) {
            const Face& face = face_q.q[i];
            if (face.contour_points.size() < 2) continue;
            float cp0 = face.contour_points[0][index];
            float cp1 = face.contour_points[1][index];
            if ((cp0 > coord_val && cp1 <= coord_val) ||
                (cp1 > coord_val && cp0 <= coord_val)) {
                result.push_back(&face);
            }
        }
    }
    return result;
}

// ─── get_intersections_at ───────────────────────────────────────────────────
// "Get the intersections between a list of faces at a particular coord_val along index"
// Mirrors the Python get_intersections function from infill.py
inline vector<Vertex> get_intersections_at(
        const vector<const Face*>& faces,
        float coord_val,
        int index) {
    Axis axis;
    if (index == 0) axis = Axis::X;
    else if (index == 1) axis = Axis::Y;
    else axis = Axis::Z;

    vector<Vertex> intersections;
    for (const Face* face : faces) {
        Vertex pt = get_intersection(face->contour_points[0], face->contour_points[1], axis, coord_val);
        intersections.push_back(pt);
    }
    return intersections;
}

// ─── unique_vertices ────────────────────────────────────────────────────────
// Remove duplicate vertices (mirrors np.unique(axis=0))
inline vector<Vertex> unique_vertices(vector<Vertex>& verts) {
    // Sort then unique
    sort(verts.begin(), verts.end());
    verts.erase(unique(verts.begin(), verts.end()), verts.end());
    return verts;
}

// ─── fill_across_index ──────────────────────────────────────────────────────
// "Fills a polygon across `index` in G-code"
// Mirrors the Python fill_across_index function from infill.py
inline void fill_across_index(
        GCodeWriter& g,
        const vector<FaceQueue>& layer_qs,
        int index,
        float current_val,
        int order_axes_by,
        float extrusion_rate,
        float& total_extruded,
        float& total_distance) {

    auto faces_at_val = find_faces_at_index(layer_qs, current_val, index);
    if (faces_at_val.empty()) return;

    auto intersections = get_intersections_at(faces_at_val, current_val, index);
    intersections = unique_vertices(intersections);
    if (intersections.size() <= 1) return;

    // Sort by the correct index/axis
    sort(intersections.begin(), intersections.end(),
        [order_axes_by](const Vertex& a, const Vertex& b) {
            return a[order_axes_by] < b[order_axes_by];
        });

    // Assert even number of intersections
    if (intersections.size() % 2 != 0) {
        throw runtime_error("len(intersections) should be even but isn't. Something's funky...");
    }

    for (size_t i = 0; i < intersections.size(); i += 2) {
        // Move to starting point
        g.abs_move(intersections[i].x, intersections[i].y, intersections[i].z, /*rapid=*/true);

        // Extrude across distance
        total_distance += distance_between(intersections[i], intersections[i + 1]);
        total_extruded = extrusion_rate * total_distance;
        g.abs_move(intersections[i + 1].x, intersections[i + 1].y, intersections[i + 1].z,
                   /*rapid=*/false, /*F=*/-1, /*E=*/total_extruded);
    }
}

// ─── gap_fill ───────────────────────────────────────────────────────────────
// "Fill a polygon with a gap in between the lines that fill it."
// Mirrors the Python gap_fill function from infill.py
inline void gap_fill(
        GCodeWriter& g,
        const vector<FaceQueue>& layer_qs,
        int index,
        float start_val,
        float end_val,
        float extrusion_rate,
        float& total_extruded,
        float& total_distance,
        float gap) {
    int order_axes_by = (index + 1) % 2;

    for (float current_val = start_val + gap; current_val < end_val; current_val += gap) {
        fill_across_index(g, layer_qs, index, current_val, order_axes_by,
                          extrusion_rate, total_extruded, total_distance);
    }
}

// ─── solid ──────────────────────────────────────────────────────────────────
// "Apply a solid fill using a gap fill of size `extrusion_width`"
// Mirrors the Python solid function from infill.py (uses gap=1)
inline void solid_infill(
        GCodeWriter& g,
        const vector<FaceQueue>& layer_qs,
        int index,
        float start_val,
        float end_val,
        float extrusion_rate,
        float& total_extruded,
        float& total_distance,
        float /*extrusion_width*/) {
    g.write("\n; Printing solid infill");
    gap_fill(g, layer_qs, index, start_val, end_val,
             extrusion_rate, total_extruded, total_distance, /*gap=*/1.0f);
}

// ─── criss_cross ────────────────────────────────────────────────────────────
// "Must specify either number_of_crosses or gap_between_crosses but not both."
// Mirrors the Python criss_cross function from infill.py
inline void criss_cross(
        GCodeWriter& g,
        const vector<FaceQueue>& layer_qs,
        float x_min, float x_max,
        float y_min, float y_max,
        float extrusion_rate,
        float& total_extruded,
        float& total_distance,
        float /*extrusion_width*/,
        float gap_between_crosses = -1,
        int number_of_crosses = -1) {

    float x_gap = (gap_between_crosses > 0) ? gap_between_crosses : (x_max - x_min) / static_cast<float>(number_of_crosses);
    float y_gap = (gap_between_crosses > 0) ? gap_between_crosses : (y_max - y_min) / static_cast<float>(number_of_crosses);

    g.write("\n; Printing x criss-crosses for cross infill");
    gap_fill(g, layer_qs, static_cast<int>(Axis::X), x_min, x_max,
             extrusion_rate, total_extruded, total_distance, x_gap);

    g.write("\n; Printing y criss-crosses for cross infill");
    gap_fill(g, layer_qs, static_cast<int>(Axis::Y), y_min, y_max,
             extrusion_rate, total_extruded, total_distance, y_gap);
}
