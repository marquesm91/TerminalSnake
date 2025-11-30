#ifndef PLATFORM_NCURSES_H_
#define PLATFORM_NCURSES_H_

#include <ncurses.h>
#include <chrono>
#include "../core/engine.hpp"

namespace SnakePlatform {

class NCursesPlatform : public SnakeCore::IPlatform {
private:
    uint8_t boardWidth;
    uint8_t boardHeight;
    std::chrono::steady_clock::time_point startTime;
    int highscore;
    
    // Color pair definitions
    static constexpr int COLOR_BORDER = 1;
    static constexpr int COLOR_SNAKE_HEAD = 2;
    static constexpr int COLOR_SNAKE_BODY = 3;
    static constexpr int COLOR_FOOD = 4;
    static constexpr int COLOR_SCORE = 5;
    static constexpr int COLOR_GAMEOVER = 7;

public:
    NCursesPlatform() : boardWidth(80), boardHeight(24), highscore(0) {
        startTime = std::chrono::steady_clock::now();
    }
    
    void init() {
        initscr();
        start_color();
        cbreak();
        use_default_colors();
        curs_set(0);
        keypad(stdscr, TRUE);
        noecho();
        nodelay(stdscr, TRUE);
        
        // Initialize colors
        init_pair(COLOR_BORDER, COLOR_CYAN, -1);
        init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, -1);
        init_pair(COLOR_SNAKE_BODY, COLOR_GREEN, -1);
        init_pair(COLOR_FOOD, COLOR_RED, -1);
        init_pair(COLOR_SCORE, COLOR_YELLOW, -1);
        init_pair(COLOR_GAMEOVER, COLOR_RED, -1);
        
        // Get terminal dimensions
        boardHeight = static_cast<uint8_t>(LINES);
        boardWidth = static_cast<uint8_t>(COLS);
    }
    
    void shutdown() {
        endwin();
    }
    
    void setHighscore(int hs) { highscore = hs; }
    
    // IPlatform implementation
    
    uint32_t getTimeMs() override {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        return static_cast<uint32_t>(duration.count());
    }
    
    void delay(uint16_t ms) override {
        napms(ms);
    }
    
    int8_t getInput() override {
        int ch = getch();
        switch (ch) {
            case KEY_UP:    return SnakeCore::DIR_UP;
            case KEY_DOWN:  return SnakeCore::DIR_DOWN;
            case KEY_LEFT:  return SnakeCore::DIR_LEFT;
            case KEY_RIGHT: return SnakeCore::DIR_RIGHT;
            default:        return SnakeCore::DIR_NONE;
        }
    }
    
    void clear() override {
        ::clear();
    }
    
    void drawSnakeHead(int16_t x, int16_t y) override {
        attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
        mvaddch(x, y, 'O');
        attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    }
    
    void drawSnakeBody(int16_t x, int16_t y) override {
        attron(COLOR_PAIR(COLOR_SNAKE_BODY));
        mvaddch(x, y, 'o');
        attroff(COLOR_PAIR(COLOR_SNAKE_BODY));
    }
    
    void drawFood(int16_t x, int16_t y) override {
        attron(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
        mvaddch(x, y, ACS_DIAMOND);
        attroff(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
    }
    
    void drawWall(int16_t x, int16_t y) override {
        attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
        
        // Determine wall character based on position
        if (x == 0 && y == 0) {
            mvaddch(x, y, ACS_ULCORNER);
        } else if (x == 0 && y == boardWidth - 1) {
            mvaddch(x, y, ACS_URCORNER);
        } else if (x == boardHeight - 1 && y == 0) {
            mvaddch(x, y, ACS_LLCORNER);
        } else if (x == boardHeight - 1 && y == boardWidth - 1) {
            mvaddch(x, y, ACS_LRCORNER);
        } else if (x == 0 || x == boardHeight - 1) {
            mvaddch(x, y, ACS_HLINE);
        } else {
            mvaddch(x, y, ACS_VLINE);
        }
        
        attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    }
    
    void drawScore(uint32_t score, uint16_t size) override {
        // Status bar at top
        attron(COLOR_PAIR(COLOR_SCORE) | A_BOLD);
        mvprintw(0, 2, " SCORE: %d ", score);
        mvprintw(0, 18, " SIZE: %d ", size);
        mvprintw(0, boardWidth - 20, " HIGHSCORE: %d ", highscore);
        attroff(COLOR_PAIR(COLOR_SCORE) | A_BOLD);
    }
    
    void drawGameOver(uint32_t score) override {
        int centerY = boardHeight / 2;
        int centerX = boardWidth / 2;
        
        // Draw box
        int boxWidth = 40;
        int boxHeight = 10;
        int boxY = centerY - boxHeight / 2;
        int boxX = centerX - boxWidth / 2;
        
        attron(COLOR_PAIR(COLOR_GAMEOVER));
        
        // Fill box
        for (int i = 0; i < boxHeight; i++) {
            for (int j = 0; j < boxWidth; j++) {
                mvaddch(boxY + i, boxX + j, ' ');
            }
        }
        
        attron(A_BOLD);
        mvprintw(boxY + 2, centerX - 5, "GAME OVER");
        mvprintw(boxY + 4, centerX - 8, "Final Score: %d", score);
        mvprintw(boxY + 6, centerX - 12, "Press Y to play again");
        mvprintw(boxY + 7, centerX - 8, "Press N to exit");
        attroff(A_BOLD);
        
        attroff(COLOR_PAIR(COLOR_GAMEOVER));
    }
    
    void refresh() override {
        ::refresh();
    }
    
    uint8_t getBoardWidth() override { return boardWidth; }
    uint8_t getBoardHeight() override { return boardHeight; }
    
    // Wait for Y/N input after game over
    char waitForPlayAgain() {
        nodelay(stdscr, FALSE);  // Blocking mode
        char ch;
        do {
            ch = toupper(getch());
        } while (ch != 'Y' && ch != 'N' && ch != '\n');
        nodelay(stdscr, TRUE);  // Back to non-blocking
        return ch;
    }
};

} // namespace SnakePlatform

#endif
