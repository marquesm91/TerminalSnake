#ifndef BOARD_H_
#define BOARD_H_ 

#include <string>

using namespace std;

// Color pair definitions
#define COLOR_BORDER 1
#define COLOR_SNAKE_HEAD 2
#define COLOR_SNAKE_BODY 3
#define COLOR_FOOD 4
#define COLOR_SCORE 5
#define COLOR_HIGHSCORE 6
#define COLOR_GAMEOVER 7
#define COLOR_STATUS_BG 8

class Board {

    int score;
    int highscore;
    int gameAreaTop;    // Where the game area starts (after status bar)

    void drawStatusBar() {
        // Draw a stylized status bar at the top
        attron(COLOR_PAIR(COLOR_STATUS_BG) | A_BOLD);
        for (int j = 0; j < COLS; j++) {
            mvaddch(0, j, ' ');
        }
        attroff(COLOR_PAIR(COLOR_STATUS_BG) | A_BOLD);

        // Score section
        attron(COLOR_PAIR(COLOR_SCORE) | A_BOLD);
        mvprintw(0, 2, " SCORE ");
        attroff(A_BOLD);
        attron(COLOR_PAIR(COLOR_SCORE));
        printw(" %d ", score);
        attroff(COLOR_PAIR(COLOR_SCORE));

        // Size section
        attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
        mvprintw(0, 18, " SIZE ");
        attroff(A_BOLD);
        attron(COLOR_PAIR(COLOR_SNAKE_HEAD));
        printw(" 3 ");
        attroff(COLOR_PAIR(COLOR_SNAKE_HEAD));

        // Highscore section (right aligned)
        attron(COLOR_PAIR(COLOR_HIGHSCORE) | A_BOLD);
        mvprintw(0, COLS - 20, " HIGHSCORE ");
        attroff(A_BOLD);
        attron(COLOR_PAIR(COLOR_HIGHSCORE));
        printw(" %d ", highscore);
        attroff(COLOR_PAIR(COLOR_HIGHSCORE));
    }

    void drawGameBorder() {
        attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
        
        // Top border with corners
        mvaddch(gameAreaTop, 0, ACS_ULCORNER);
        mvaddch(gameAreaTop, COLS - 1, ACS_URCORNER);
        for (int j = 1; j < COLS - 1; j++) {
            mvaddch(gameAreaTop, j, ACS_HLINE);
        }

        // Bottom border with corners
        mvaddch(LINES - 1, 0, ACS_LLCORNER);
        mvaddch(LINES - 1, COLS - 1, ACS_LRCORNER);
        for (int j = 1; j < COLS - 1; j++) {
            mvaddch(LINES - 1, j, ACS_HLINE);
        }

        // Side borders
        for (int i = gameAreaTop + 1; i < LINES - 1; i++) {
            mvaddch(i, 0, ACS_VLINE);
            mvaddch(i, COLS - 1, ACS_VLINE);
        }

        attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    }

public:

    Board(int initialHighscore = 0) { 

        score = 0;
        highscore = initialHighscore;
        gameAreaTop = 1;

        drawStatusBar();
        drawGameBorder();
    }

    int getScore() const { return score; }
    
    void setPrintHighscore(int newHighscore) {
        highscore = newHighscore;
        attron(COLOR_PAIR(COLOR_HIGHSCORE));
        mvprintw(0, COLS - 8, " %d   ", highscore);
        attroff(COLOR_PAIR(COLOR_HIGHSCORE));
    }

    void update() {
        refresh();
    }

    char getChar(const Point &p) const {
        chtype ch = mvinch(p.getX(), p.getY());
        char c = ch & A_CHARTEXT;
        // Check for snake body (different chars now)
        if (c == 'O' || c == 'o' || c == '@') return '@';
        // Check for borders (ACS characters)
        if ((ch & A_CHARTEXT) == (ACS_HLINE & A_CHARTEXT) || 
            (ch & A_CHARTEXT) == (ACS_VLINE & A_CHARTEXT) ||
            (ch & A_CHARTEXT) == (ACS_ULCORNER & A_CHARTEXT) ||
            (ch & A_CHARTEXT) == (ACS_URCORNER & A_CHARTEXT) ||
            (ch & A_CHARTEXT) == (ACS_LLCORNER & A_CHARTEXT) ||
            (ch & A_CHARTEXT) == (ACS_LRCORNER & A_CHARTEXT) ||
            c == '-' || c == '|') return '-';
        // Check for food (ACS_DIAMOND)
        if ((ch & A_CHARTEXT) == (ACS_DIAMOND & A_CHARTEXT)) return 'f';
        return c;
    }

    void setPrintFood(const Point &f) {
        // Draw food with color and special character
        attron(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
        mvaddch(f.getX(), f.getY(), ACS_DIAMOND);
        attroff(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
    }

    void setPrintSnake(const Body &b) { 
        // Draw snake head with bright color
        attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
        mvprintw(b.getHead().getX(), b.getHead().getY(), "O");
        attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

        // Clear tail position
        mvprintw(b.getTail().getX(), b.getTail().getY(), " ");
    }

    void setPrintScore(int level) {
        score = score + level;
        attron(COLOR_PAIR(COLOR_SCORE));
        mvprintw(0, 10, " %d   ", score - level);
        attroff(COLOR_PAIR(COLOR_SCORE));
    }

    void setPrintSize(const Body &b) { 
        attron(COLOR_PAIR(COLOR_SNAKE_HEAD));
        mvprintw(0, 25, " %d ", b.getSize());
        attroff(COLOR_PAIR(COLOR_SNAKE_HEAD));
    }

    void printGameOver(){
        // Draw semi-transparent overlay effect
        attron(COLOR_PAIR(COLOR_GAMEOVER));
        
        int boxWidth = 50;
        int boxHeight = 15;
        int startY = (LINES / 2) - (boxHeight / 2);
        int startX = (COLS / 2) - (boxWidth / 2);

        // Draw box background
        for (int i = 0; i < boxHeight; i++) {
            for (int j = 0; j < boxWidth; j++) {
                mvaddch(startY + i, startX + j, ' ');
            }
        }

        // Draw box border
        attron(A_BOLD);
        mvaddch(startY, startX, ACS_ULCORNER);
        mvaddch(startY, startX + boxWidth - 1, ACS_URCORNER);
        mvaddch(startY + boxHeight - 1, startX, ACS_LLCORNER);
        mvaddch(startY + boxHeight - 1, startX + boxWidth - 1, ACS_LRCORNER);
        
        for (int j = 1; j < boxWidth - 1; j++) {
            mvaddch(startY, startX + j, ACS_HLINE);
            mvaddch(startY + boxHeight - 1, startX + j, ACS_HLINE);
        }
        for (int i = 1; i < boxHeight - 1; i++) {
            mvaddch(startY + i, startX, ACS_VLINE);
            mvaddch(startY + i, startX + boxWidth - 1, ACS_VLINE);
        }

        // Game Over text (ASCII art style)
        string gameOver[5];
        gameOver[0] = " ####   ###  #   # ####";
        gameOver[1] = "#      #   # ## ## #   ";
        gameOver[2] = "#  ## ##### # # # ###  ";
        gameOver[3] = "#   # #   # #   # #    ";
        gameOver[4] = " ###  #   # #   # #### ";

        string overText[5];
        overText[0] = " ###  #   # #### ####";
        overText[1] = "#   # #   # #    #   #";
        overText[2] = "#   #  # #  ###  ####";
        overText[3] = "#   #  # #  #    #  #";
        overText[4] = " ###    #   #### #   #";

        int textStartY = startY + 2;
        int textStartX = startX + 3;

        attron(COLOR_PAIR(COLOR_GAMEOVER) | A_BOLD);
        for (int i = 0; i < 5; i++) {
            mvprintw(textStartY + i, textStartX, "%s", gameOver[i].c_str());
            mvprintw(textStartY + i, textStartX + 25, "%s", overText[i].c_str());
        }

        // Final score display
        attroff(COLOR_PAIR(COLOR_GAMEOVER));
        attron(COLOR_PAIR(COLOR_SCORE) | A_BOLD);
        mvprintw(startY + 8, startX + (boxWidth / 2) - 10, "FINAL SCORE: %d", score);
        attroff(COLOR_PAIR(COLOR_SCORE));

        attron(COLOR_PAIR(COLOR_HIGHSCORE) | A_BOLD);
        mvprintw(startY + 9, startX + (boxWidth / 2) - 10, "HIGHSCORE: %d", highscore);
        attroff(COLOR_PAIR(COLOR_HIGHSCORE));

        // New highscore message
        if (score > highscore) {
            attron(COLOR_PAIR(COLOR_FOOD) | A_BOLD | A_BLINK);
            mvprintw(startY + 11, startX + (boxWidth / 2) - 10, "** NEW HIGHSCORE! **");
            attroff(COLOR_PAIR(COLOR_FOOD) | A_BOLD | A_BLINK);
        }

        // Retry prompt
        attron(COLOR_PAIR(COLOR_BORDER));
        mvprintw(startY + boxHeight - 2, startX + (boxWidth / 2) - 10, "Play again? (Y/n)");
        attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    }

    void printObstacle(const Point &p) {
        attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
        mvaddch(p.getX(), p.getY(), ACS_CKBOARD); // Use checkerboard pattern for obstacles
        attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    }

};

#endif