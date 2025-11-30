# Makefile for WebAssembly build of Terminal Snake
# Requires: Emscripten SDK (emcc)

# Compiler - use system emcc or local emsdk
EMCC := $(shell which emcc 2>/dev/null || echo "$(HOME)/apps/TerminalSnake/emsdk/upstream/emscripten/emcc")

# Directories
SRC_DIR = .
BUILD_DIR = build
OUTPUT_DIR = ../firebase/public

# Output files
WASM_OUTPUT = $(OUTPUT_DIR)/tsnake.js

# Source files
SOURCES = main_web.cpp

# Exported functions
EXPORTS = ["_main","_startGame","_runGameLoop","_handleKey"]

# Compiler flags for release build
EMCC_FLAGS = -O2 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createSnakeModule" \
    -s EXPORTED_FUNCTIONS='$(EXPORTS)' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s ENVIRONMENT=web \
    -s NO_EXIT_RUNTIME=1 \
    -s ALLOW_MEMORY_GROWTH=0 \
    -s INITIAL_MEMORY=2097152 \
    -s STACK_SIZE=131072

.PHONY: all clean setup check-emcc

all: check-emcc setup $(WASM_OUTPUT)

check-emcc:
@$(EMCC) --version > /dev/null 2>&1 || (echo "Error: Emscripten not found. Run 'source emsdk/emsdk_env.sh'" && exit 1)

setup:
@mkdir -p $(BUILD_DIR)
@mkdir -p $(OUTPUT_DIR)

$(WASM_OUTPUT): $(SOURCES)
$(EMCC) $(SOURCES) $(EMCC_FLAGS) -o $(WASM_OUTPUT)
@echo "Build complete: $(WASM_OUTPUT)"
@ls -la $(OUTPUT_DIR)/tsnake.*

clean:
rm -rf $(BUILD_DIR)
rm -f $(OUTPUT_DIR)/tsnake.js $(OUTPUT_DIR)/tsnake.wasm
