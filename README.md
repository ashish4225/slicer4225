# slicer4225

A fast C++ slicer that converts `.obj` 3D models into `.gcode` for 3D printing.

> C++ port of [sliceofpy](https://github.com/your-repo/sliceofpy) with identical output.

---

## Quick Start

```bash
# 1. Build
make

# 2. Slice an OBJ file
make slice FILE=tests/block.obj

# 3. Done! Your G-code is at block.gcode
```

That's it. Three commands.

---

## Commands

| Command | What it does |
|---|---|
| `make` | Build the project |
| `make slice FILE=tests/block.obj` | Slice a model → G-code |
| `make slice FILE=tests/block.obj OUT=myprint.gcode` | Slice with a custom output name |
| `make test` | Run the test suite |
| `make clean` | Remove build artifacts |
| `make help` | Show available commands |

---

## Examples

```bash
# Slice the included test models
make slice FILE=tests/block.obj
make slice FILE=tests/pyramid.obj
make slice FILE=tests/icecream.obj
make slice FILE=tests/ring.obj
make slice FILE=tests/torus.obj

# Custom output filename
make slice FILE=tests/block.obj OUT=my_block.gcode
```

---

## Advanced Usage

If you need fine-grained control, you can call the binary directly after building:

```bash
./build/slicer4225 model.obj [options]
```

### Options

| Flag | Description | Default |
|---|---|---|
| `-o, --output <file>` | Output G-code file | `out.gcode` |
| `-l, --layer_height <mm>` | Layer height | `0.2` |
| `-s, --scale <factor>` | Scale factor | `1.0` |
| `-f, --feedrate <mm/min>` | Max feedrate | `3600` |
| `--feedrate_writing <mm/min>` | Writing feedrate | `feedrate/2` |
| `-d, --filament_diameter <mm>` | Filament diameter | `1.75` |
| `-w, --extrusion_width <mm>` | Extrusion width | `0.4` |
| `--extrusion_multiplier <f>` | Extrusion multiplier | `1.0` |
| `--misc_infill <type>` | Infill: `cross`, `solid`, `none` | `cross` |
| `--gap_between_crosses <mm>` | Gap for cross infill | `5.0` |
| `--num_solid_fill <n>` | Solid layers top/bottom | `3` |
| `-t, --temperature <val>` | Nozzle temp or material (`PLA`, `ABS`) | `PLA` |
| `--bed_temperature <val>` | Bed temp or material | `PLA` |
| `--units <mm\|in>` | Units | `mm` |
| `--base_offset <mm>` | Base Z offset | `layer_height/2` |
| `--template_dir <dir>` | G-code templates directory | `./templates/` |

### Example with options

```bash
./build/slicer4225 tests/block.obj -o block.gcode -l 0.3 -s 2.0 -t ABS
```

---

## Requirements

- CMake ≥ 3.14
- C++17 compiler (GCC 7+, Clang 5+)
- GNU Make
