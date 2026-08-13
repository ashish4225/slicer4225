# slicer4225

A fast, command-line 3D object slicer written in C++. It takes a `.obj` file as input and outputs a `.gcode` file ready for 3D printing.

This is a pure C++ port of the original Python-based `sliceofpy` project, with identical slicing logic and  G-code output.

## Building

Requires CMake (>= 3.14) and a C++17 compliant compiler.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This will produce two executables:
- `slicer4225`: The main command-line slicer.
- `slicer4225_tests`: The test suite.

## Usage

```bash
./build/slicer4225 <filename.obj> [options]
```

### Options:

- `-o, --output <file>`: Output file (default: `out.gcode`)
- `-l, --layer_height <float>`: Layer height in mm (default: 0.2)
- `-s, --scale <float>`: Scale factor (default: 1.0)
- `-f, --feedrate <float>`: Max feedrate mm/min (default: 3600)
- `--feedrate_writing <float>`: Writing feedrate mm/min (default: `feedrate/2`)
- `-d, --filament_diameter <float>`: Filament diameter (default: 1.75)
- `-w, --extrusion_width <float>`: Extrusion width (default: 0.4)
- `--extrusion_multiplier <float>`: Extrusion multiplier (default: 1.0)
- `--misc_infill <string>`: Infill type: `cross`, `solid`, `none` (default: `cross`)
- `--gap_between_crosses <float>`: Gap for cross infill (default: 5.0)
- `--num_solid_fill <int>`: Solid layers top/bottom (default: 3)
- `-t, --temperature <string>`: Nozzle temp or material name (e.g. `PLA`, `ABS`) (default: `PLA`)
- `--bed_temperature <string>`: Bed temp or material name (default: `PLA`)
- `--units <string>`: Units: `mm` or `in` (default: `mm`)
- `--base_offset <float>`: Base Z offset (default: `layer_height/2`)
- `--template_dir <dir>`: Path to gcode templates directory (default: `./templates/`)

## Testing

To run the test suite:

```bash
./build/slicer4225_tests tests/ templates/
```
