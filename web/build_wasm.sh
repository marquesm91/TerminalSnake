#!/bin/bash
# build_wasm.sh - Build Terminal Snake WebAssembly version
# This script installs Emscripten if needed and builds the WASM module

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EMSDK_DIR="$PROJECT_ROOT/emsdk"

echo "🐍 Terminal Snake - WASM Build Script"
echo "======================================"

# Check if emsdk exists
if [ ! -d "$EMSDK_DIR" ]; then
    echo ""
    echo "📦 Emscripten SDK not found. Installing..."
    echo ""
    
    cd "$PROJECT_ROOT"
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    
    echo ""
    echo "🔧 Installing latest Emscripten..."
    ./emsdk install latest
    
    echo ""
    echo "✅ Activating Emscripten..."
    ./emsdk activate latest
fi

# Source emsdk environment
echo ""
echo "🔄 Loading Emscripten environment..."
source "$EMSDK_DIR/emsdk_env.sh"

# Verify emcc is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: emcc not found after sourcing emsdk_env.sh"
    exit 1
fi

echo "✅ Emscripten version: $(emcc --version | head -n1)"

# Build WASM
echo ""
echo "🔨 Building WebAssembly module..."
cd "$SCRIPT_DIR"

make clean 2>/dev/null || true
make all

echo ""
echo "✅ Build complete!"
echo ""
echo "📁 Generated files:"
ls -la "$PROJECT_ROOT/firebase/public/tsnake."* 2>/dev/null || echo "   (check firebase/public/)"

echo ""
echo "🚀 To test locally:"
echo "   cd $PROJECT_ROOT/firebase/public"
echo "   python3 -m http.server 8080"
echo "   # Open http://localhost:8080/wasm.html"
echo ""
echo "📤 To deploy to Firebase:"
echo "   cd $PROJECT_ROOT/firebase"
echo "   firebase deploy --only hosting"
