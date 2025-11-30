# Terminal Snake - Web Version (WebAssembly)

This directory contains the WebAssembly build of Terminal Snake - the **real C++ game** compiled to run in your browser!

## 🎮 Quick Start

```bash
# 1. Install Emscripten SDK (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..

# 2. Build WASM
cd web
make

# 3. Serve locally (from firebase/public)
cd ../firebase/public
python3 -m http.server 8080

# 4. Open in browser
# http://localhost:8080/wasm.html
```

## 📁 Files

| File | Description |
|------|-------------|
| `main_web.cpp` | Emscripten entry point with simplified game loop |
| `web_curses.hpp` | ncurses-compatible API that bridges to xterm.js |
| `Makefile` | Build configuration for Emscripten |
| `README.md` | This file |

## 🔧 Build Options

```bash
make            # Release build (optimized)
make debug      # Debug build with symbols
make clean      # Remove build artifacts
make local      # Build to local directory
make help       # Show all targets
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Browser                                 │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐   │
│  │   xterm.js  │ ←── │  tsnake.js  │ ←── │ tsnake.wasm │   │
│  │  (Terminal) │     │ (Emscripten │     │  (C++ Game) │   │
│  │             │     │   Glue)     │     │             │   │
│  └─────────────┘     └─────────────┘     └─────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                     wasm.html                                │
└─────────────────────────────────────────────────────────────┘
```

### How it Works

1. **web_curses.hpp** provides ncurses-compatible functions (`initscr`, `getch`, `mvprintw`, etc.)
2. These functions use `EM_JS` macros to call JavaScript functions
3. JavaScript functions write to xterm.js terminal emulator
4. Keyboard input is captured and queued for `getch()`
5. The game loop runs at 60fps via `emscripten_set_main_loop`

## 🎨 Color Support

The web version supports the same color pairs as the native version:

| Pair | Foreground | Usage |
|------|------------|-------|
| 1 | Cyan | Border |
| 2 | Green | Snake head |
| 3 | Green | Snake body |
| 4 | Red | Food |
| 5 | Yellow | Score |
| 6 | Magenta | Level |
| 7 | Red | Game Over |

## 🕹️ Controls

- **Arrow Keys** or **WASD**: Move snake
- **R**: Restart game after game over

Mobile users get on-screen buttons.

## 📦 Deploy to Firebase

```bash
# Build WASM
cd web
make

# Deploy
cd ../firebase
firebase deploy --only hosting
```

The WASM files (`tsnake.js` and `tsnake.wasm`) are output directly to `firebase/public/`.

## 🐛 Troubleshooting

### "WASM module not found"
Run `make` in the `web/` directory first.

### "Emscripten not found"
Make sure you've activated emsdk:
```bash
source /path/to/emsdk/emsdk_env.sh
```

### Terminal doesn't render correctly
Try a different browser. Chrome and Firefox work best.

### Game is too fast/slow
The game runs at 60fps with frame-skipping based on level. Adjust `updateDelay` in `main_web.cpp`.

## 📚 Dependencies

- **Emscripten** 3.0+ (C++ to WASM compiler)
- **xterm.js** 5.3+ (terminal emulator, loaded via CDN)
- Modern browser with WebAssembly support
