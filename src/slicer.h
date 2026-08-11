#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>

#include "math_utils.h"
#include "face.h"
#include "obj_parser.h"
#include "gcode_writer.h"
#include "infill.h"

using namespace std;

static const map<string, float> material_nozzle_temps = {
    {"PLA", 215.0f},
    {"ABS", 235.0f},
};

static const map<string, float> material_bed_temps = {
    {"PLA", 60.0f},
    {"ABS", 100.0f},
};

inline float process_temp(const string& temp, const map<string, float>& lookup) {
    try { return stof(temp); } catch (...) {}
    string upper_temp = temp;
    for (auto& c : upper_temp) c = static_cast<char>(toupper(c));
    auto it = lookup.find(upper_temp);
    if (it != lookup.end()) return it->second;
    throw runtime_error("Temperature not recognized: " + temp);
}

inline float vertices_axis_min(const vector<Vertex>& vertices, int axis) {
    float mn = numeric_limits<float>::max();
    for (const auto& v : vertices) { if (v[axis] < mn) mn = v[axis]; }
    return mn;
}

inline float vertices_axis_max(const vector<Vertex>& vertices, int axis) {
    float mx = -numeric_limits<float>::max();
    for (const auto& v : vertices) { if (v[axis] > mx) mx = v[axis]; }
    return mx;
}

inline void generate_contours(
        const string& filename, float layer_height, float scale, float base_offset,
        vector<vector<FaceQueue>>& face_qs_out, vector<Vertex>& vertices_out) {

    vector<vector<int>> raw_faces;
    parse_obj(filename, raw_faces, vertices_out);
    float z_max = center_vertices(vertices_out, base_offset);
    int num_slices = static_cast<int>(ceil((z_max - base_offset) * scale / layer_height));
    cerr << "INFO: Number of slices: " << num_slices << endl;
    face_qs_out.clear();

    for (int i = 0; i < num_slices; ++i) {
        float zi = static_cast<float>(i) * layer_height + base_offset;
        vector<FaceQueue> layer_fqs;
        FaceQueue face_q;
        layer_fqs.push_back(face_q);

        for (size_t face_num = 0; face_num < raw_faces.size(); ++face_num) {
            const auto& face_indices = raw_faces[face_num];
            vector<Vertex> current_verts;
            for (int idx : face_indices) current_verts.push_back(vertices_out[idx]);

            vector<Vertex> lowers, uppers;
            for (const auto& v : current_verts) {
                if (v.z <= zi) lowers.push_back(v);
                else uppers.push_back(v);
            }

            if (!lowers.empty() && !uppers.empty()) {
                Face f_class(face_indices, static_cast<int>(face_num));
                for (const auto& low_vert : lowers)
                    for (const auto& upp_vert : uppers)
                        f_class.add_contour_pts(get_intersection(low_vert, upp_vert, Axis::Z, zi));

                FaceQueue& active_fq = layer_fqs.back();
                bool isFqFull = active_fq.insert(f_class);

                if (isFqFull && (!active_fq.store.empty() || (face_num < raw_faces.size() - 1))) {
                    FaceQueue extra_face_q;
                    for (const auto& f : active_fq.store) extra_face_q.insert(f);
                    active_fq.store.clear();
                    layer_fqs.push_back(extra_face_q);
                } else if (face_num == raw_faces.size() - 1) {
                    // Mirror Python: append current face_q at end
                    // In Python this creates a duplicate reference;
                    // in C++ we copy the current back() to match behavior.
                    layer_fqs.push_back(layer_fqs.back());
                }
            }
        }
        face_qs_out.push_back(layer_fqs);
    }
}

inline void generate_gcode(
        const string& filename, const string& outfile = "out.gcode",
        float layer_height = 0.2f, float scale = 1.0f, float feedrate = 3600.0f,
        float feedrate_writing = -1.0f, float filament_diameter = 1.75f,
        float extrusion_width = 0.4f, float extrusion_multiplier = 1.0f,
        const string& misc_infill = "cross", float gap_between_crosses = 5.0f,
        int num_solid_fill = 3, const string& temperature = "PLA",
        const string& bed_temperature = "PLA", const string& units = "mm",
        float base_offset = 0.1f, const string& template_dir = "./templates/") {

    vector<vector<FaceQueue>> face_qs;
    vector<Vertex> vertices;
    generate_contours(filename, layer_height, scale, base_offset, face_qs, vertices);

    if (feedrate_writing < 0) feedrate_writing = static_cast<int>(feedrate) / 2.0f;

    float flow_area = extrusion_multiplier * extrusion_width * layer_height;
    float flowrate = flow_area * feedrate_writing / 60.0f;
    float extrusion_rate = flow_area / (filament_diameter * filament_diameter / 4.0f * static_cast<float>(M_PI));
    cerr << "INFO: The flowrate is set to " << flowrate << "mm^3/s" << endl;

    float nozzle_temp = process_temp(temperature, material_nozzle_temps);
    float bed_temp = process_temp(bed_temperature, material_bed_temps);
    cerr << "INFO: The nozzle temperature is set to " << nozzle_temp << " degrees celsius" << endl;
    cerr << "INFO: The bed temperature is set to " << bed_temp << " degrees celsius" << endl;

    float total_distance = 0, total_extruded = 0;

    auto ftos = [](float f) -> string { ostringstream ss; ss << f; return ss.str(); };

    string units_str = (units == "in") ? "0 \t\t\t\t\t;use inches" : "1 \t\t\t\t\t;use mm";

    process_gcode_template(template_dir + "header.gcode", "header.tmp",
        {{"units", units_str}, {"feedrate", ftos(feedrate)},
         {"temperature", ftos(nozzle_temp)}, {"bed_temperature", ftos(bed_temp)}});
    process_gcode_template(template_dir + "footer.gcode", "footer.tmp",
        {{"feedrate", ftos(feedrate)}});

    GCodeWriter g(outfile, filament_diameter, layer_height, "header.tmp", "footer.tmp", vertices);
    g.open();
    g.absolute();

    for (size_t layer_num = 0; layer_num < face_qs.size(); ++layer_num) {
        const auto& layer_qs = face_qs[layer_num];
        g.write("\n; Printing layer " + to_string(layer_num));
        g.write("; ====================");
        g.write("\n; Printing outline");

        for (const auto& contour : layer_qs) {
            Vertex start_pt, last_pt, next_pt;
            for (size_t i = 0; i < contour.q.size(); ++i) {
                const Face& face = contour.q[i];
                if (face.contour_points.size() < 2) continue;

                if (i == 0) {
                    if (contour.q.size() > 1 && contour.q[1].contour_points.size() >= 2 &&
                        (face.contour_points[0] == contour.q[1].contour_points[0] ||
                         face.contour_points[0] == contour.q[1].contour_points[1])) {
                        start_pt = face.contour_points[1];
                        next_pt  = face.contour_points[0];
                    } else {
                        start_pt = face.contour_points[0];
                        next_pt  = face.contour_points[1];
                    }
                    g.abs_move(start_pt.x, start_pt.y, start_pt.z, true, feedrate);
                    last_pt = start_pt;
                } else {
                    next_pt = (face.contour_points[0] == last_pt) ? face.contour_points[1] : face.contour_points[0];
                }

                float distance = distance_between(next_pt, last_pt);
                total_distance += distance;
                float extrusion_amount = extrusion_rate * distance;
                g.abs_move(next_pt.x, next_pt.y, next_pt.z, false, feedrate_writing, total_extruded + extrusion_amount);
                total_extruded += extrusion_amount;
                last_pt = next_pt;
            }

            if (contour.q.empty()) continue;
            float distance = distance_between(start_pt, last_pt);
            total_distance += distance;
            float extrusion_amount = extrusion_rate * distance;
            g.abs_move(start_pt.x, start_pt.y, start_pt.z, false, feedrate_writing, total_extruded + extrusion_amount);
            total_extruded += extrusion_amount;
        }

        if (static_cast<int>(layer_num) < num_solid_fill ||
            layer_num >= face_qs.size() - static_cast<size_t>(num_solid_fill) ||
            misc_infill == "solid") {
            int axis = (layer_num % 2 == 0) ? static_cast<int>(Axis::X) : static_cast<int>(Axis::Y);
            solid_infill(g, layer_qs, axis, vertices_axis_min(vertices, axis),
                         vertices_axis_max(vertices, axis), extrusion_rate, total_extruded, total_distance, extrusion_width);
        } else if (misc_infill == "cross") {
            criss_cross(g, layer_qs, vertices_axis_min(vertices, static_cast<int>(Axis::X)),
                        vertices_axis_max(vertices, static_cast<int>(Axis::X)),
                        vertices_axis_min(vertices, static_cast<int>(Axis::Y)),
                        vertices_axis_max(vertices, static_cast<int>(Axis::Y)),
                        extrusion_rate, total_extruded, total_distance, extrusion_width, gap_between_crosses);
        }
    }

    cerr << "INFO: Total nozzle distance: " << total_distance << "mm" << endl;
    cerr << "INFO: Estimated filament used: " << total_extruded << "mm" << endl;
    g.close();
}
