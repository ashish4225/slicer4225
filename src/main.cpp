// ─── sliceofpy C++ CLI ──────────────────────────────────────────────────────
// Mirrors the Python cli.py — "A command line object slicer for .obj files."
//
// Usage:
//   ./sliceofpy <filename.obj> [options]
//
// All flags mirror the Python argparse interface.

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "slicer.h"

using namespace std;

struct CliArgs {
    string filename;
    string output         = "out.gcode";
    float layer_height         = 0.2f;
    float scale                = 1.0f;
    float feedrate             = 3600.0f;
    float feedrate_writing     = -1.0f;
    float filament_diameter    = 1.75f;
    float extrusion_width      = 0.4f;
    float extrusion_multiplier = 1.0f;
    string misc_infill    = "cross";
    float gap_between_crosses  = 5.0f;
    int num_solid_fill         = 3;
    string temperature    = "PLA";
    string bed_temperature = "PLA";
    string units          = "mm";
    float base_offset          = -1.0f;
    string template_dir   = "";
};

static void print_usage(const char* prog) {
    cout << "Usage: " << prog << " <filename.obj> [options]\n"
         << "\nA command line object slicer for .obj files.\n"
         << "\nOptions:\n"
         << "  -o, --output <file>              Output file (default: out.gcode)\n"
         << "  -l, --layer_height <float>       Layer height in mm (default: 0.2)\n"
         << "  -s, --scale <float>              Scale factor (default: 1.0)\n"
         << "  -f, --feedrate <float>           Max feedrate mm/min (default: 3600)\n"
         << "      --feedrate_writing <float>   Writing feedrate mm/min (default: feedrate/2)\n"
         << "  -d, --filament_diameter <float>  Filament diameter (default: 1.75)\n"
         << "  -w, --extrusion_width <float>    Extrusion width (default: 0.4)\n"
         << "      --extrusion_multiplier <f>   Extrusion multiplier (default: 1.0)\n"
         << "      --misc_infill <str>          Infill type: cross, solid, none (default: cross)\n"
         << "      --gap_between_crosses <f>    Gap for cross infill (default: 5.0)\n"
         << "      --num_solid_fill <int>       Solid layers top/bottom (default: 3)\n"
         << "  -t, --temperature <str>          Nozzle temp or material name (default: PLA)\n"
         << "      --bed_temperature <str>      Bed temp or material name (default: PLA)\n"
         << "      --units <str>                Units: mm or in (default: mm)\n"
         << "      --base_offset <float>        Base Z offset (default: layer_height/2)\n"
         << "      --template_dir <dir>         Path to gcode templates dir\n"
         << "  -h, --help                       Show this help message\n";
}

static CliArgs parse_args(int argc, char* argv[]) {
    CliArgs args;

    if (argc < 2) { print_usage(argv[0]); exit(1); }

    args.filename = argv[1];
    if (args.filename == "-h" || args.filename == "--help") { print_usage(argv[0]); exit(0); }

    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        auto next = [&]() -> string {
            if (i + 1 >= argc) { cerr << "ERROR: Missing value for " << arg << endl; exit(1); }
            return argv[++i];
        };

        if (arg == "-o" || arg == "--output")                  args.output = next();
        else if (arg == "-l" || arg == "--layer_height")       args.layer_height = stof(next());
        else if (arg == "-s" || arg == "--scale")              args.scale = stof(next());
        else if (arg == "-f" || arg == "--feedrate")           args.feedrate = stof(next());
        else if (arg == "--feedrate_writing")                  args.feedrate_writing = stof(next());
        else if (arg == "-d" || arg == "--filament_diameter")  args.filament_diameter = stof(next());
        else if (arg == "-w" || arg == "--extrusion_width")    args.extrusion_width = stof(next());
        else if (arg == "--extrusion_multiplier")              args.extrusion_multiplier = stof(next());
        else if (arg == "--misc_infill")                       args.misc_infill = next();
        else if (arg == "--gap_between_crosses")               args.gap_between_crosses = stof(next());
        else if (arg == "--num_solid_fill")                    args.num_solid_fill = stoi(next());
        else if (arg == "-t" || arg == "--temperature")        args.temperature = next();
        else if (arg == "--bed_temperature")                   args.bed_temperature = next();
        else if (arg == "--units")                             args.units = next();
        else if (arg == "--base_offset")                       args.base_offset = stof(next());
        else if (arg == "--template_dir")                      args.template_dir = next();
        else if (arg == "-h" || arg == "--help")               { print_usage(argv[0]); exit(0); }
        else { cerr << "ERROR: Unknown argument: " << arg << endl; print_usage(argv[0]); exit(1); }
    }
    return args;
}

int main(int argc, char* argv[]) {
    CliArgs args = parse_args(argc, argv);

    if (args.base_offset < 0) args.base_offset = args.layer_height / 2.0f;
    if (args.template_dir.empty()) args.template_dir = "./templates/";

    try {
        generate_gcode(args.filename, args.output, args.layer_height, args.scale,
            args.feedrate, args.feedrate_writing, args.filament_diameter,
            args.extrusion_width, args.extrusion_multiplier, args.misc_infill,
            args.gap_between_crosses, args.num_solid_fill, args.temperature,
            args.bed_temperature, args.units, args.base_offset, args.template_dir);
        cout << "G-code written to: " << args.output << endl;
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
    return 0;
}
