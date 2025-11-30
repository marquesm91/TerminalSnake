# Makefile for WebAssembly build of Terminal Snake
# Requires: Emscripten SDK (emcc)

# Compiler
EMCC = emcc

# Directories
SRC_DIR = .
BUILD_DIR = build
OUTPUT_DIR = ../firebase/public

# Output files
WASM_OUTPUT = $(OUTPUT_DIR)/tsnake.js
HTML_SHELL = shell.html

# Source files
SOURCES = main_web.cpp

# Compiler flags
EMCC_FLAGS = \
    -std=c++17 \
    -O2 \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createSnakeModule" \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ASYNCIFY=1 \
    -s NO_EXIT_RUNTIME=1 \
    -I$(SRC_DIR) \
    -I../libs

# Debug flags (add -g for debugging)
DEBUG_FLAGS = -g -s ASSERTIONS=2

# Release flags
RELEASE_FLAGS = -O3 --closure 1

.PHONY: all clean debug release setup check-emcc

all: check-emcc setup release

check-emcc:
	@which emcc > /dev/null 2>&1 || (echo "Error: Emscripten not found. Please install and activate emsdk." && exit 1)

setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUTPUT_DIR)

debug: check-emcc setup
	$(EMCC) $(SOURCES) $(EMCC_FLAGS) $(DEBUG_FLAGS) -o $(WASM_OUTPUT)
	@echo "Debug build complete: $(WASM_OUTPUT)"

release: check-emcc setup
	$(EMCC) $(SOURCES) $(EMCC_FLAGS) $(RELEASE_FLAGS) -o $(WASM_OUTPUT)
	@echo "Release build complete: $(WASM_OUTPUT)"
	@echo "Files generated:"
	@ls -la $(OUTPUT_DIR)/tsnake.*

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OUTPUT_DIR)/tsnake.js
	rm -f $(OUTPUT_DIR)/tsnake.wasm

# Build for local testing (outputs to local directory)
local: check-emcc
	@mkdir -p $(BUILD_DIR)
	$(EMCC) $(SOURCES) $(EMCC_FLAGS) -o $(BUILD_DIR)/tsnake.js
	@cp $(BUILD_DIR)/tsnake.js $(BUILD_DIR)/tsnake.wasm . 2>/dev/null || true
	@echo "Local build complete in $(BUILD_DIR)/"

# Install Emscripten (helper target)
install-emsdk:
	@echo "Installing Emscripten SDK..."
	@echo "Run the following commands:"
	@echo ""
	@echo "  git clone https://github.com/emscripten-core/emsdk.git"
	@echo "  cd emsdk"
	@echo "  ./emsdk install latest"
	@echo "  ./emsdk activate latest"
	@echo "  source ./emsdk_env.sh"
	@echo ""
	@echo "Then run 'make' again."

help:
	@echo "Terminal Snake - WebAssembly Build"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build release version (default)"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized version"
	@echo "  local    - Build to local directory for testing"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  - Emscripten SDK (emcc)"
	@echo "  - Run 'make install-emsdk' for installation instructions"
