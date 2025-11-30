/**
 * @file main_web.cpp
 * @brief WebAssembly entry point for Terminal Snake
 * 
 * This compiles the ORIGINAL game code to WebAssembly.
 * Uses web_curses.hpp as a drop-in replacement for ncurses.
 */

// Use web_curses instead of ncurses
#include "web_curses.hpp"

// Include original game components
#include "../libs/common.hpp"
#include "../libs/point.hpp"
#include "../libs/clock.hpp"
#include "../libs/food.hpp"
#include "../libs/body.hpp"
#include "../libs/board.hpp"
#include "../libs/game.hpp"

#include <emscripten.h>

// ============================================================================
// Global Game State
// ============================================================================

static Game* g = nullptr;
static int currentLevel = 1;
static bool gameRunning = false;

// ============================================================================
// Exported Functions
// ============================================================================

extern "C" {

/**
 * Initialize the game
 */
EMSCRIPTEN_KEEPALIVE
void initGame(int level) {
    if (g) {
        delete g;
    }
    
    // Initialize ncurses (web version)
    initscr();
    start_color();
    cbreak();
    use_default_colors();
    curs_set(0);
    keypad(stdscr, TRUE);
    noecho();
    nodelay(stdscr, TRUE);
    
    // Initialize colors (same as ncurses_platform.hpp)
    init_pair(COLOR_BORDER, COLOR_CYAN, COLOR_DEFAULT);
    init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, COLOR_DEFAULT);
    init_pair(COLOR_SNAKE_BODY, COLOR_GREEN, COLOR_DEFAULT);
    init_pair(COLOR_FOOD, COLOR_RED, COLOR_DEFAULT);
    init_pair(COLOR_SCORE, COLOR_YELLOW, COLOR_DEFAULT);
    init_pair(COLOR_HIGHSCORE, COLOR_MAGENTA, COLOR_DEFAULT);
    init_pair(COLOR_GAMEOVER, COLOR_RED, COLOR_DEFAULT);
    init_pair(COLOR_STATUS_BG, COLOR_WHITE, COLOR_BLUE);
    
    currentLevel = level;
    g = new Game(level);
    gameRunning = true;
}

/**
 * Process one game frame
 * Returns: 0 if game continues, 1 if game over
 */
EMSCRIPTEN_KEEPALIVE
int gameFrame() {
    if (!g || !gameRunning) {
        return 1;
    }
    
    // isGameOver() does everything: reads input, moves snake, checks collisions
    if (g->isGameOver()) {
        gameRunning = false;
        return 1;
    }
    
    return 0;
}

/**
 * Handle keyboard input
 */
EMSCRIPTEN_KEEPALIVE
void handleInput(int key) {
    if (!g) return;
    
    // Push key to the input queue
    web_curses_push_key(key);
}

/**
 * Get the current score
 */
EMSCRIPTEN_KEEPALIVE
int getScore() {
    return g ? g->getScore() : 0;
}

/**
 * Get the snake size
 */
EMSCRIPTEN_KEEPALIVE  
int getSnakeSize() {
    return g ? g->getSnakeSize() : 0;
}

/**
 * Check if game is over
 */
EMSCRIPTEN_KEEPALIVE
int isGameOver() {
    return (!g || !gameRunning) ? 1 : 0;
}

/**
 * Reset the game
 */
EMSCRIPTEN_KEEPALIVE
void resetGame() {
    if (g) {
        g->reset();
        gameRunning = true;
    }
}

/**
 * Cleanup
 */
EMSCRIPTEN_KEEPALIVE
void cleanup() {
    if (g) {
        delete g;  // Game destructor calls endwin()
        g = nullptr;
    }
}

} // extern "C"

// Main entry point (not used in WASM, but needed for compilation)
int main() {
    return 0;
}
