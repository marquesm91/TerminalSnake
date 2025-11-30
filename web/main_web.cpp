/**
 * @file main_web.cpp
 * @brief WebAssembly entry point for Terminal Snake
 * 
 * This is a simplified version of main.cpp that runs in the browser
 * using Emscripten and xterm.js for terminal emulation.
 * 
 * Uses simple C arrays instead of std::list to avoid WASM initialization issues.
 */

#include <emscripten.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

// ============================================================================
// Constants
// ============================================================================

const int SCREEN_COLS = 60;
const int SCREEN_ROWS = 22;
const int MAX_SNAKE_LEN = 500;
const int GAME_AREA_TOP = 1;

// Direction constants
const int DIR_UP = 0;
const int DIR_DOWN = 1;
const int DIR_LEFT = 2;
const int DIR_RIGHT = 3;

// ============================================================================
// JavaScript Interface (using EM_JS)
// ============================================================================

EM_JS(void, js_clear, (), {
    if (typeof window !== 'undefined' && window.terminalClear) {
        window.terminalClear();
    }
});

EM_JS(void, js_mvprintw, (int y, int x, const char* str), {
    if (typeof window !== 'undefined' && window.terminalWrite) {
        window.terminalWrite(y, x, UTF8ToString(str));
    }
});

EM_JS(void, js_refresh, (), {
    if (typeof window !== 'undefined' && window.terminalRefresh) {
        window.terminalRefresh();
    }
});

EM_JS(int, js_getch, (), {
    if (typeof window !== 'undefined' && window.getKey) {
        return window.getKey();
    }
    return -1;
});

// ============================================================================
// Game State Structure
// ============================================================================

struct Point {
    int x;
    int y;
};

struct GameState {
    // Snake body as simple arrays
    Point snake[MAX_SNAKE_LEN];
    int snakeLen;
    
    // Food position
    Point food;
    
    // Direction: DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
    int direction;
    int disableDirection;
    
    // Game stats
    int score;
    int level;
    int frameCount;
    
    // Game state flags
    bool gameOver;
    bool initialized;
    
    // Screen buffer for collision detection
    char screen[SCREEN_ROWS][SCREEN_COLS];
};

// Global game state (zero-initialized)
static GameState game = {};

// ============================================================================
// Utility Functions
// ============================================================================

void clearScreen() {
    for (int y = 0; y < SCREEN_ROWS; y++) {
        for (int x = 0; x < SCREEN_COLS; x++) {
            game.screen[y][x] = ' ';
        }
    }
}

void placeFood() {
    // Find a random valid position for food
    int attempts = 0;
    do {
        game.food.x = 1 + (rand() % (SCREEN_COLS - 2));
        game.food.y = GAME_AREA_TOP + 1 + (rand() % (SCREEN_ROWS - GAME_AREA_TOP - 2));
        attempts++;
    } while (game.screen[game.food.y][game.food.x] != ' ' && attempts < 100);
}

// ============================================================================
// Drawing Functions
// ============================================================================

void drawBorder() {
    // Top and bottom borders
    char topBot[SCREEN_COLS + 1];
    memset(topBot, '#', SCREEN_COLS);
    topBot[SCREEN_COLS] = '\0';
    
    js_mvprintw(GAME_AREA_TOP, 0, topBot);
    js_mvprintw(SCREEN_ROWS - 1, 0, topBot);
    
    // Mark in buffer
    for (int x = 0; x < SCREEN_COLS; x++) {
        game.screen[GAME_AREA_TOP][x] = '#';
        game.screen[SCREEN_ROWS - 1][x] = '#';
    }
    
    // Side borders
    for (int y = GAME_AREA_TOP + 1; y < SCREEN_ROWS - 1; y++) {
        js_mvprintw(y, 0, "#");
        js_mvprintw(y, SCREEN_COLS - 1, "#");
        game.screen[y][0] = '#';
        game.screen[y][SCREEN_COLS - 1] = '#';
    }
}

void drawStatusBar() {
    char status[SCREEN_COLS + 1];
    snprintf(status, sizeof(status), "SCORE: %-6d  SIZE: %-4d  LEVEL: %d    [WASM BUILD]", 
             game.score, game.snakeLen, game.level);
    js_mvprintw(0, 0, status);
}

void drawFood() {
    js_mvprintw(game.food.y, game.food.x, "*");
    game.screen[game.food.y][game.food.x] = '*';
}

void drawSnake() {
    // Draw body
    for (int i = 1; i < game.snakeLen; i++) {
        js_mvprintw(game.snake[i].y, game.snake[i].x, "o");
        game.screen[game.snake[i].y][game.snake[i].x] = 'o';
    }
    
    // Draw head
    js_mvprintw(game.snake[0].y, game.snake[0].x, "O");
    game.screen[game.snake[0].y][game.snake[0].x] = '@';
}

void drawGameOver() {
    int centerY = SCREEN_ROWS / 2;
    int centerX = SCREEN_COLS / 2 - 10;
    
    js_mvprintw(centerY - 1, centerX, "+--------------------+");
    js_mvprintw(centerY,     centerX, "|     GAME OVER!     |");
    
    char scoreLine[32];
    snprintf(scoreLine, sizeof(scoreLine), "|  Score: %-6d     |", game.score);
    js_mvprintw(centerY + 1, centerX, scoreLine);
    
    js_mvprintw(centerY + 2, centerX, "|  Press R to retry  |");
    js_mvprintw(centerY + 3, centerX, "+--------------------+");
}

// ============================================================================
// Game Logic
// ============================================================================

void initSnake() {
    // Initialize snake in the middle
    game.snakeLen = 3;
    int startX = SCREEN_COLS / 2;
    int startY = SCREEN_ROWS / 2;
    
    for (int i = 0; i < game.snakeLen; i++) {
        game.snake[i].x = startX - i;
        game.snake[i].y = startY;
    }
    
    game.direction = DIR_RIGHT;
    game.disableDirection = DIR_LEFT;
}

void initGame() {
    // Initialize random seed
    srand(42);
    
    // Clear screen
    clearScreen();
    
    // Initialize game state
    game.score = 0;
    game.level = 1;
    game.frameCount = 0;
    game.gameOver = false;
    
    // Initialize snake
    initSnake();
    
    // Draw initial state
    js_clear();
    drawBorder();
    drawStatusBar();
    
    // Place and draw snake
    for (int i = 0; i < game.snakeLen; i++) {
        game.screen[game.snake[i].y][game.snake[i].x] = (i == 0) ? '@' : 'o';
    }
    drawSnake();
    
    // Place and draw food
    placeFood();
    drawFood();
    
    js_refresh();
    
    game.initialized = true;
}

void setDirection(int newDir) {
    if (newDir != game.disableDirection) {
        game.direction = newDir;
        switch (newDir) {
            case DIR_UP:    game.disableDirection = DIR_DOWN;  break;
            case DIR_DOWN:  game.disableDirection = DIR_UP;    break;
            case DIR_LEFT:  game.disableDirection = DIR_RIGHT; break;
            case DIR_RIGHT: game.disableDirection = DIR_LEFT;  break;
        }
    }
}

void processInput() {
    int key = js_getch();
    
    if (key == -1) return;
    
    if (game.gameOver) {
        if (key == 'r' || key == 'R') {
            initGame();
        }
        return;
    }
    
    // Map keys to directions
    switch (key) {
        case 'w': case 'W': case 1000: setDirection(DIR_UP);    break;
        case 's': case 'S': case 1001: setDirection(DIR_DOWN);  break;
        case 'a': case 'A': case 1002: setDirection(DIR_LEFT);  break;
        case 'd': case 'D': case 1003: setDirection(DIR_RIGHT); break;
    }
}

void updateGame() {
    if (game.gameOver) return;
    
    // Calculate new head position
    Point newHead = game.snake[0];
    switch (game.direction) {
        case DIR_UP:    newHead.y--; break;
        case DIR_DOWN:  newHead.y++; break;
        case DIR_LEFT:  newHead.x--; break;
        case DIR_RIGHT: newHead.x++; break;
    }
    
    // Check collision with walls and self
    char cell = game.screen[newHead.y][newHead.x];
    if (cell == '#' || cell == '@' || cell == 'o') {
        game.gameOver = true;
        return;
    }
    
    // Check if eating food
    bool eating = (cell == '*');
    
    // Clear tail from screen (if not eating)
    if (!eating) {
        int tailIdx = game.snakeLen - 1;
        js_mvprintw(game.snake[tailIdx].y, game.snake[tailIdx].x, " ");
        game.screen[game.snake[tailIdx].y][game.snake[tailIdx].x] = ' ';
    }
    
    // Move snake body
    if (eating) {
        // Grow snake - shift everything back and add new head
        if (game.snakeLen < MAX_SNAKE_LEN) {
            for (int i = game.snakeLen; i > 0; i--) {
                game.snake[i] = game.snake[i - 1];
            }
            game.snakeLen++;
        }
        
        // Update score
        game.score += game.level * 10;
        
        // Level up every 50 points
        game.level = 1 + game.score / 50;
        if (game.level > 10) game.level = 10;
        
        // Place new food
        placeFood();
        drawFood();
        
        // Update status bar
        drawStatusBar();
    } else {
        // Normal move - shift positions
        for (int i = game.snakeLen - 1; i > 0; i--) {
            game.snake[i] = game.snake[i - 1];
        }
    }
    
    // Update head position
    game.snake[0] = newHead;
    
    // Draw snake
    for (int i = 0; i < game.snakeLen; i++) {
        if (i == 0) {
            js_mvprintw(game.snake[i].y, game.snake[i].x, "O");
            game.screen[game.snake[i].y][game.snake[i].x] = '@';
        } else {
            js_mvprintw(game.snake[i].y, game.snake[i].x, "o");
            game.screen[game.snake[i].y][game.snake[i].x] = 'o';
        }
    }
}

void gameLoop() {
    game.frameCount++;
    
    // Process all pending input
    processInput();
    
    if (game.gameOver) {
        drawGameOver();
        js_refresh();
        return;
    }
    
    // Only update game at certain intervals based on level
    // Higher level = faster game
    int updateInterval = 12 - game.level;
    if (updateInterval < 3) updateInterval = 3;
    
    if (game.frameCount % updateInterval == 0) {
        updateGame();
    }
    
    js_refresh();
}

// ============================================================================
// Exported Functions for JavaScript
// ============================================================================

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void startGame() {
        initGame();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void runGameLoop() {
        gameLoop();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void handleKey(int key) {
        if (game.gameOver) {
            if (key == 'r' || key == 'R') {
                initGame();
            }
            return;
        }
        
        switch (key) {
            case 'w': case 'W': case 1000: setDirection(DIR_UP);    break;
            case 's': case 'S': case 1001: setDirection(DIR_DOWN);  break;
            case 'a': case 'A': case 1002: setDirection(DIR_LEFT);  break;
            case 'd': case 'D': case 1003: setDirection(DIR_RIGHT); break;
        }
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================

int main() {
    // Don't auto-start - let JavaScript call startGame()
    return 0;
}
