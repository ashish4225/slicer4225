# ─── slicer4225 Makefile ──────────────────────────────────────────────────────
# Simple wrapper around CMake so you only need to type `make` and `make slice`.

BUILD_DIR := build

.PHONY: all build clean slice test help

## Default: build everything
all: build

## Build the project (creates build/ if needed)
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. -Wno-dev 2>/dev/null && cmake --build . -j$$(nproc)
	@echo ""
	@echo "✅  Build complete!"
	@echo "    Executable: $(BUILD_DIR)/slicer4225"

## Slice an OBJ file → G-code  (usage: make slice FILE=model.obj)
slice:
ifndef FILE
	@echo "Usage:  make slice FILE=path/to/model.obj [OUT=output.gcode]"
	@echo ""
	@echo "Example:"
	@echo "  make slice FILE=tests/block.obj"
	@echo "  make slice FILE=tests/block.obj OUT=block.gcode"
	@exit 1
endif
	@$(BUILD_DIR)/slicer4225 $(FILE) -o $(or $(OUT),$(basename $(notdir $(FILE))).gcode) --template_dir templates/

## Run the test suite
test: build
	@$(BUILD_DIR)/slicer4225_tests tests/ templates/

## Remove build artifacts
clean:
	@rm -rf $(BUILD_DIR)
	@echo "🧹  Cleaned."

## Show help
help:
	@echo ""
	@echo "slicer4225 — 3D Object Slicer"
	@echo "─────────────────────────────────────────"
	@echo ""
	@echo "  make              Build the project"
	@echo "  make slice FILE=model.obj    Slice an OBJ → G-code"
	@echo "  make test         Run the test suite"
	@echo "  make clean        Remove build artifacts"
	@echo "  make help         Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make slice FILE=tests/block.obj"
	@echo "  make slice FILE=tests/block.obj OUT=myprint.gcode"
	@echo ""
