/**
 * @file main_web.cpp
 * @brief WebAssembly entry point for Terminal Snake
 * 
 * This is a simplified version of main.cpp that runs in the browser
 * using Emscripten and xterm.js for terminal emulation.
 */

#include <emscripten.h>
#include <emscripten/html5.h>

// Use our web-compatible ncurses replacement
#include "web_curses.hpp"

// Include game components (they use ncurses functions now mapped to web_curses)
#include "../libs/common.hpp"
#include "../libs/point.hpp"
#include "../libs/clock.hpp"

// ============================================================================
// Simplified game classes for web (avoiding complex includes)
// ============================================================================

// Simple food class
class WebFood : public Point {
public:
    WebFood() {}
    
    void getFood() {
        // Simple random using emscripten_random
        int x = 2 + static_cast<int>(emscripten_random() * (LINES - 4));
        int y = 1 + static_cast<int>(emscripten_random() * (COLS - 3));
        this->x = x;
        this->y = y;
    }
};

// Simple body class
#include <list>
class WebBody {
private:
    std::list<Point> body;
    int direction;
    int disableDirection;

public:
    WebBody() {
        body.push_front(Point(5, 5));
        body.push_front(Point(5, 6));
        body.push_front(Point(5, 7));
        validateDirection(RIGHT);
    }

    void validateDirection(int dir) {
        if (dir != ERR && dir != disableDirection && dir >= 2 && dir <= 5) {
            this->direction = dir;
            switch (this->direction) {
                case UP: disableDirection = DOWN; break;
                case DOWN: disableDirection = UP; break;
                case LEFT: disableDirection = RIGHT; break;
                case RIGHT: disableDirection = LEFT; break;
            }
        }
    }

    Point investigatePosition() {
        Point newHead;
        switch (this->direction) {
            case UP: 
                newHead.setX(getHead().getX() - 1);
                newHead.setY(getHead().getY()); 
                break;
            case DOWN: 
                newHead.setX(getHead().getX() + 1);
                newHead.setY(getHead().getY()); 
                break;
            case LEFT: 
                newHead.setX(getHead().getX()); 
                newHead.setY(getHead().getY() - 1); 
                break;
            case RIGHT:	
                newHead.setX(getHead().getX()); 
                newHead.setY(getHead().getY() + 1); 
                break;
        }
        return newHead;
    }

    Point getHead() const { return body.front(); }
    void setHead(const Point& p) { body.push_front(p); }
    Point getTail() const { return body.back(); }
    void removeTail() { body.pop_back(); }
    int getSize() const { return body.size(); }
    int getDirection() const { return direction; }
};

// ============================================================================
// Game State
// ============================================================================

struct GameState {
    WebBody* body;
    WebFood* food;
    int score;
    int level;
    bool gameOver;
    bool initialized;
    int frameCount;
    int gameAreaTop;
    
    // Screen buffer for collision detection
    char screenBuffer[25][80];
    
    GameState() : body(nullptr), food(nullptr), score(0), level(1), 
                  gameOver(false), initialized(false), frameCount(0), gameAreaTop(1) {
        // Clear screen buffer
        for (int i = 0; i < 25; i++) {
            for (int j = 0; j < 80; j++) {
                screenBuffer[i][j] = ' ';
            }
        }
    }
};

static GameState gameState;

// ============================================================================
// Drawing Functions
// ============================================================================

void drawBorder() {
    attron(COLOR_PAIR(1));  // Cyan
    
    int top = gameState.gameAreaTop;
    
    // Top border
    mvaddch(top, 0, '+');
    mvaddch(top, COLS - 1, '+');
    for (int j = 1; j < COLS - 1; j++) {
        mvaddch(top, j, '-');
        gameState.screenBuffer[top][j] = '-';
    }
    
    // Bottom border
    mvaddch(LINES - 1, 0, '+');
    mvaddch(LINES - 1, COLS - 1, '+');
    for (int j = 1; j < COLS - 1; j++) {
        mvaddch(LINES - 1, j, '-');
        gameState.screenBuffer[LINES - 1][j] = '-';
    }
    
    // Side borders
    for (int i = top + 1; i < LINES - 1; i++) {
        mvaddch(i, 0, '|');
        mvaddch(i, COLS - 1, '|');
        gameState.screenBuffer[i][0] = '|';
        gameState.screenBuffer[i][COLS - 1] = '|';
    }
    
    // Corners
    gameState.screenBuffer[top][0] = '+';
    gameState.screenBuffer[top][COLS - 1] = '+';
    gameState.screenBuffer[LINES - 1][0] = '+';
    gameState.screenBuffer[LINES - 1][COLS - 1] = '+';
    
    attroff(COLOR_PAIR(1));
}

void drawStatusBar() {
    attron(COLOR_PAIR(5));  // Yellow
    mvprintw(0, 2, "SCORE: %d", gameState.score);
    attroff(COLOR_PAIR(5));
    
    attron(COLOR_PAIR(2));  // Green
    mvprintw(0, 20, "SIZE: %d", gameState.body ? gameState.body->getSize() : 3);
    attroff(COLOR_PAIR(2));
    
    attron(COLOR_PAIR(6));  // Magenta
    mvprintw(0, 35, "LEVEL: %d", gameState.level);
    attroff(COLOR_PAIR(6));
    
    attron(COLOR_PAIR(1));  // Cyan
    mvprintw(0, COLS - 20, "WASM BUILD");
    attroff(COLOR_PAIR(1));
}

void drawFood() {
    if (!gameState.food) return;
    
    int x = gameState.food->getX();
    int y = gameState.food->getY();
    
    // Clear old food position if any
    attron(COLOR_PAIR(4));  // Red
    mvaddch(x, y, '*');
    attroff(COLOR_PAIR(4));
    
    gameState.screenBuffer[x][y] = 'f';
}

void drawSnake() {
    if (!gameState.body) return;
    
    Point head = gameState.body->getHead();
    Point tail = gameState.body->getTail();
    
    // Draw head
    attron(COLOR_PAIR(2));  // Green
    mvaddch(head.getX(), head.getY(), 'O');
    attroff(COLOR_PAIR(2));
    
    gameState.screenBuffer[head.getX()][head.getY()] = '@';
    
    // Clear tail (only if not eating)
    mvaddch(tail.getX(), tail.getY(), ' ');
    gameState.screenBuffer[tail.getX()][tail.getY()] = ' ';
}

void drawGameOver() {
    int centerY = LINES / 2;
    int centerX = COLS / 2;
    
    attron(COLOR_PAIR(7));  // Red
    mvprintw(centerY - 2, centerX - 10, "+--------------------+");
    mvprintw(centerY - 1, centerX - 10, "|     GAME OVER!     |");
    mvprintw(centerY,     centerX - 10, "|  Score: %-6d     |", gameState.score);
    mvprintw(centerY + 1, centerX - 10, "|  Press R to retry  |");
    mvprintw(centerY + 2, centerX - 10, "+--------------------+");
    attroff(COLOR_PAIR(7));
}

void validateFood() {
    if (!gameState.food) return;
    
    gameState.food->getFood();
    
    int x = gameState.food->getX();
    int y = gameState.food->getY();
    
    // Check if food spawned on snake or border
    if (x >= 0 && x < 25 && y >= 0 && y < 80) {
        char ch = gameState.screenBuffer[x][y];
        if (ch == '@' || ch == '-' || ch == '|' || ch == '+') {
            validateFood();  // Try again
        }
    }
}

// ============================================================================
// Game Logic
// ============================================================================

void initGame() {
    initscr();
    start_color();
    use_default_colors();
    curs_set(0);
    nodelay(stdscr, TRUE);
    noecho();
    cbreak();
    
    // Initialize color pairs
    init_pair(1, COLOR_CYAN, COLOR_DEFAULT);     // Border
    init_pair(2, COLOR_GREEN, COLOR_DEFAULT);    // Snake
    init_pair(3, COLOR_GREEN, COLOR_DEFAULT);    // Snake body
    init_pair(4, COLOR_RED, COLOR_DEFAULT);      // Food
    init_pair(5, COLOR_YELLOW, COLOR_DEFAULT);   // Score
    init_pair(6, COLOR_MAGENTA, COLOR_DEFAULT);  // Level
    init_pair(7, COLOR_RED, COLOR_DEFAULT);      // Game over
    
    // Clear screen buffer
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            gameState.screenBuffer[i][j] = ' ';
        }
    }
    
    clear();
    
    gameState.body = new WebBody();
    gameState.food = new WebFood();
    gameState.score = 0;
    gameState.gameOver = false;
    gameState.frameCount = 0;
    
    validateFood();
    
    drawBorder();
    drawStatusBar();
    drawFood();
    
    // Draw initial snake
    attron(COLOR_PAIR(2));
    mvaddch(5, 5, 'o');
    mvaddch(5, 6, 'o');
    mvaddch(5, 7, 'O');
    attroff(COLOR_PAIR(2));
    
    gameState.screenBuffer[5][5] = '@';
    gameState.screenBuffer[5][6] = '@';
    gameState.screenBuffer[5][7] = '@';
    
    refresh();
    
    gameState.initialized = true;
}

void resetGame() {
    if (gameState.body) {
        delete gameState.body;
    }
    if (gameState.food) {
        delete gameState.food;
    }
    
    gameState.initialized = false;
    initGame();
}

void gameLoop() {
    if (!gameState.initialized || gameState.gameOver) {
        // Check for restart
        int key = getch();
        if (key == 'r' || key == 'R') {
            resetGame();
        }
        return;
    }
    
    gameState.frameCount++;
    
    // Only update every N frames (based on level)
    int updateDelay = 8 - gameState.level;
    if (updateDelay < 2) updateDelay = 2;
    
    if (gameState.frameCount % updateDelay != 0) {
        // Still check for input
        int key = getch();
        if (key != ERR) {
            switch (key) {
                case KEY_UP:
                case 'w':
                case 'W':
                    gameState.body->validateDirection(UP);
                    break;
                case KEY_DOWN:
                case 's':
                case 'S':
                    gameState.body->validateDirection(DOWN);
                    break;
                case KEY_LEFT:
                case 'a':
                case 'A':
                    gameState.body->validateDirection(LEFT);
                    break;
                case KEY_RIGHT:
                case 'd':
                case 'D':
                    gameState.body->validateDirection(RIGHT);
                    break;
            }
        }
        return;
    }
    
    // Process input
    int key = getch();
    if (key != ERR) {
        switch (key) {
            case KEY_UP:
            case 'w':
            case 'W':
                gameState.body->validateDirection(UP);
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                gameState.body->validateDirection(DOWN);
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                gameState.body->validateDirection(LEFT);
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                gameState.body->validateDirection(RIGHT);
                break;
        }
    }
    
    // Get next position
    Point newHead = gameState.body->investigatePosition();
    int x = newHead.getX();
    int y = newHead.getY();
    
    // Check collision
    if (x >= 0 && x < 25 && y >= 0 && y < 80) {
        char ch = gameState.screenBuffer[x][y];
        
        if (ch == '@' || ch == '-' || ch == '|' || ch == '+') {
            // Collision! Game over
            gameState.gameOver = true;
            drawGameOver();
            refresh();
            return;
        }
        
        if (ch == 'f') {
            // Eat food
            gameState.score += gameState.level * 10;
            
            // Move snake (don't remove tail = grow)
            gameState.body->setHead(newHead);
            gameState.screenBuffer[x][y] = '@';
            
            // Draw new head
            attron(COLOR_PAIR(2));
            mvaddch(x, y, 'O');
            attroff(COLOR_PAIR(2));
            
            // New food
            validateFood();
            drawFood();
            drawStatusBar();
        } else {
            // Normal move
            Point tail = gameState.body->getTail();
            
            // Clear tail from screen
            mvaddch(tail.getX(), tail.getY(), ' ');
            gameState.screenBuffer[tail.getX()][tail.getY()] = ' ';
            
            // Move snake
            gameState.body->setHead(newHead);
            gameState.body->removeTail();
            
            // Draw new head
            gameState.screenBuffer[x][y] = '@';
            attron(COLOR_PAIR(2));
            mvaddch(x, y, 'O');
            attroff(COLOR_PAIR(2));
        }
    } else {
        // Out of bounds
        gameState.gameOver = true;
        drawGameOver();
    }
    
    refresh();
}

// ============================================================================
// Emscripten Main Loop
// ============================================================================

void mainLoop() {
    gameLoop();
}

int main() {
    initGame();
    
    // Set up the Emscripten main loop
    // Run at 60 FPS
    emscripten_set_main_loop(mainLoop, 60, 1);
    
    return 0;
}
