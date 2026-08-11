#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include "math_utils.h"

using namespace std;

// ─── GCodeWriter ────────────────────────────────────────────────────────────
// Mirrors the Python G wrapper class from draw.py
// Writes G-code commands to an output file with header/footer support.
class GCodeWriter {
public:
    string outfile;
    float filament_diameter;
    float layer_height;
    string header_file;
    string footer_file;

    // Current position tracking
    float X, Y, Z;

    // Bounding box from vertices (for potential plotting)
    float x_min, y_min, z_min;
    float x_max, y_max, z_max;

private:
    ofstream ofs;

public:
    GCodeWriter(const string& _outfile,
                float _filament_diameter,
                float _layer_height,
                const string& _header,
                const string& _footer,
                const vector<Vertex>& vertices)
        : outfile(_outfile)
        , filament_diameter(_filament_diameter)
        , layer_height(_layer_height)
        , header_file(_header)
        , footer_file(_footer)
        , X(0), Y(0), Z(0)
        , x_min(numeric_limits<float>::max())
        , y_min(numeric_limits<float>::max())
        , z_min(numeric_limits<float>::max())
        , x_max(-numeric_limits<float>::max())
        , y_max(-numeric_limits<float>::max())
        , z_max(-numeric_limits<float>::max())
    {
        // Compute bounding box from vertices
        for (const auto& v : vertices) {
            if (v.x < x_min) x_min = v.x;
            if (v.y < y_min) y_min = v.y;
            if (v.z < z_min) z_min = v.z;
            if (v.x > x_max) x_max = v.x;
            if (v.y > y_max) y_max = v.y;
            if (v.z > z_max) z_max = v.z;
        }
    }

    void open() {
        ofs.open(outfile);
        if (!ofs.is_open()) {
            throw runtime_error("Cannot open output file: " + outfile);
        }
        // Write header
        write_file_contents(header_file);
    }

    void close() {
        // Write footer
        write_file_contents(footer_file);
        ofs.close();
    }

    // Write a raw string/comment to the gcode file
    void write(const string& s) {
        ofs << s << "\n";
    }

    // Set absolute positioning mode
    void absolute() {
        ofs << "G90\n";
    }

    // Absolute move — mirrors g.abs_move(*pt, rapid=..., F=..., E=...)
    // The Python code calls g.abs_move(x, y, z, rapid=True/False, F=feedrate, E=extrusion)
    void abs_move(float x, float y, float z,
                  bool rapid = false,
                  float F = -1,
                  float E = -1) {
        ostringstream cmd;
        cmd << fixed << setprecision(4);

        if (rapid) {
            cmd << "G0";
        } else {
            cmd << "G1";
        }

        cmd << " X" << x << " Y" << y << " Z" << z;

        if (F > 0) {
            cmd << " F" << F;
        }
        if (E >= 0 && !rapid) {
            cmd << " E" << E;
        }

        ofs << cmd.str() << "\n";

        X = x; Y = y; Z = z;
    }

private:
    void write_file_contents(const string& filename) {
        ifstream infile(filename);
        if (!infile.is_open()) {
            // Silently skip if file doesn't exist (could be empty template)
            return;
        }
        string line;
        while (getline(infile, line)) {
            ofs << line << "\n";
        }
    }
};

// ─── process_gcode_template ─────────────────────────────────────────────────
// Process gcode template with simple {key} replacement and write into tmp file.
// Mirrors the Python process_gcode_template function.
inline void process_gcode_template(const string& template_path,
                                   const string& tmp_name,
                                   const vector<pair<string, string>>& replacements) {
    ifstream infile(template_path);
    if (!infile.is_open()) {
        throw runtime_error("Cannot open template file: " + template_path);
    }

    ostringstream content;
    content << infile.rdbuf();
    string data = content.str();

    // Replace all {key} patterns with their values
    for (const auto& kv : replacements) {
        string placeholder = "{" + kv.first + "}";
        size_t pos = 0;
        while ((pos = data.find(placeholder, pos)) != string::npos) {
            data.replace(pos, placeholder.size(), kv.second);
            pos += kv.second.size();
        }
    }

    ofstream outfile(tmp_name);
    if (!outfile.is_open()) {
        throw runtime_error("Cannot open tmp file for writing: " + tmp_name);
    }
    outfile << data;
}
