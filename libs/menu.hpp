#ifndef MENU_H_
#define MENU_H_

#include <ncurses.h>
#include <string>
#include <vector>
#include "common.hpp"

class Menu {

private:
    int selectedOption;
    int difficultyLevel;
    int animationFrame;
    std::vector<std::string> menuOptions;
    std::vector<std::string> difficultyOptions;
    bool userSignedIn;

    void drawLogo(int startY, int startX) {
        // Animated color effect for the logo
        int colorShift = animationFrame % 3;
        
        std::vector<std::string> logo = {
            "  ___  _  _   _   _  _____",
            " / __|| \\| | /_\\ | |/ / __|",
            " \\__ \\| .` |/ _ \\| ' <| _|",
            " |___/|_|\\_/_/ \\_\\_|\\_\\___|"
        };

        int logoWidth = 28;
        int logoStartX = startX - (logoWidth / 2);

        // Draw each line with animated colors
        for (size_t i = 0; i < logo.size(); i++) {
            int colorPair = ((static_cast<int>(i) + colorShift) % 3) + 1;
            if (colorPair == 1) {
                attron(COLOR_PAIR(1) | A_BOLD);  // Cyan
            } else if (colorPair == 2) {
                attron(COLOR_PAIR(2) | A_BOLD);  // Green
            } else {
                attron(COLOR_PAIR(5) | A_BOLD);  // Yellow
            }
            mvprintw(startY + static_cast<int>(i), logoStartX, "%s", logo[i].c_str());
            attroff(COLOR_PAIR(1) | A_BOLD);
            attroff(COLOR_PAIR(2) | A_BOLD);
            attroff(COLOR_PAIR(5) | A_BOLD);
        }

        // Subtitle
        attron(COLOR_PAIR(5));  // Yellow
        mvprintw(startY + 6, startX - 10, "Terminal Edition v2.0");
        attroff(COLOR_PAIR(5));
    }

    void drawMenuBox(int startY, int startX, int height, int width) {
        attron(COLOR_PAIR(1));  // Cyan border

        // Corners
        mvaddch(startY, startX, ACS_ULCORNER);
        mvaddch(startY, startX + width - 1, ACS_URCORNER);
        mvaddch(startY + height - 1, startX, ACS_LLCORNER);
        mvaddch(startY + height - 1, startX + width - 1, ACS_LRCORNER);

        // Horizontal lines
        for (int j = 1; j < width - 1; j++) {
            mvaddch(startY, startX + j, ACS_HLINE);
            mvaddch(startY + height - 1, startX + j, ACS_HLINE);
        }

        // Vertical lines
        for (int i = 1; i < height - 1; i++) {
            mvaddch(startY + i, startX, ACS_VLINE);
            mvaddch(startY + i, startX + width - 1, ACS_VLINE);
        }

        attroff(COLOR_PAIR(1));
    }

public:
    Menu() : selectedOption(0), difficultyLevel(1), animationFrame(0), userSignedIn(false) {
        menuOptions = {"Start Game", "Leaderboard", "Settings", "Sign In", "Exit"};
        difficultyOptions = {"Easy", "Normal", "Hard", "Insane"};
    }
    
    void setUserSignedIn(bool signedIn, const std::string& userName = "") {
        userSignedIn = signedIn;
        if (signedIn && !userName.empty()) {
            menuOptions[3] = "Sign Out (" + userName.substr(0, 10) + ")";
        } else {
            menuOptions[3] = "Sign In";
        }
    }

    int getDifficultyLevel() const { 
        // Return delay multiplier based on difficulty
        switch (difficultyLevel) {
            case 0: return 1;  // Easy - slower
            case 1: return 2;  // Normal
            case 2: return 3;  // Hard - faster
            case 3: return 5;  // Insane - very fast
            default: return 2;
        }
    }

    int showMainMenu(int highscore) {
        clear();
        
        // Increment animation frame
        animationFrame++;
        
        int centerY = LINES / 2;
        int centerX = COLS / 2;

        // Draw logo at top
        drawLogo(3, centerX);

        // Menu box
        int boxHeight = 16;
        int boxWidth = 30;
        int boxY = centerY - 4;
        int boxX = centerX - boxWidth / 2;

        drawMenuBox(boxY, boxX, boxHeight, boxWidth);

        // Menu title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(boxY + 1, centerX - 5, "MAIN MENU");
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Draw menu options
        for (size_t i = 0; i < menuOptions.size(); i++) {
            if (static_cast<int>(i) == selectedOption) {
                attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(1));
            }
            mvprintw(boxY + 3 + static_cast<int>(i) * 2, centerX - static_cast<int>(menuOptions[i].length()) / 2, 
                     "%s", menuOptions[i].c_str());
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            attroff(COLOR_PAIR(1));
        }

        // Highscore display
        attron(COLOR_PAIR(6) | A_BOLD);
        mvprintw(boxY + boxHeight - 2, centerX - 8, "Highscore: %d", highscore);
        attroff(COLOR_PAIR(6) | A_BOLD);

        // Instructions at bottom
        attron(COLOR_PAIR(5));
        mvprintw(LINES - 2, centerX - 20, "Use Arrow Keys to navigate, Enter to select");
        attroff(COLOR_PAIR(5));

        refresh();

        // Set timeout for animation (150ms)
        timeout(150);
        
        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selectedOption = (selectedOption - 1 + static_cast<int>(menuOptions.size())) % static_cast<int>(menuOptions.size());
                return -1;  // Stay in menu
            case KEY_DOWN:
                selectedOption = (selectedOption + 1) % static_cast<int>(menuOptions.size());
                return -1;  // Stay in menu
            case '\n':
            case KEY_ENTER:
                timeout(-1);  // Reset to blocking
                return selectedOption;
            case 'q':
            case 'Q':
                timeout(-1);  // Reset to blocking
                return 4;  // Exit
            case ERR:  // Timeout - just continue animating
                return -1;
            default:
                return -1;  // Stay in menu
        }
    }

    bool showSettings() {
        clear();

        int centerY = LINES / 2;
        int centerX = COLS / 2;

        // Settings box
        int boxHeight = 14;
        int boxWidth = 40;
        int boxY = centerY - boxHeight / 2;
        int boxX = centerX - boxWidth / 2;

        drawMenuBox(boxY, boxX, boxHeight, boxWidth);

        // Title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(boxY + 1, centerX - 4, "SETTINGS");
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Difficulty selection
        attron(COLOR_PAIR(5));
        mvprintw(boxY + 3, boxX + 3, "Difficulty:");
        attroff(COLOR_PAIR(5));

        for (size_t i = 0; i < difficultyOptions.size(); i++) {
            if (static_cast<int>(i) == difficultyLevel) {
                attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(1));
            }
            mvprintw(boxY + 5 + static_cast<int>(i), centerX - static_cast<int>(difficultyOptions[i].length()) / 2, 
                     "%s", difficultyOptions[i].c_str());
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            attroff(COLOR_PAIR(1));
        }

        // Instructions
        attron(COLOR_PAIR(5));
        mvprintw(boxY + boxHeight - 3, boxX + 5, "Up/Down: Change difficulty");
        mvprintw(boxY + boxHeight - 2, boxX + 5, "Q: Back to main menu");
        attroff(COLOR_PAIR(5));

        refresh();

        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                difficultyLevel = (difficultyLevel - 1 + static_cast<int>(difficultyOptions.size())) % static_cast<int>(difficultyOptions.size());
                return false;
            case KEY_DOWN:
                difficultyLevel = (difficultyLevel + 1) % static_cast<int>(difficultyOptions.size());
                return false;
            case 'q':
            case 'Q':
            case '\n':
                return true;  // Exit settings
            default:
                return false;
        }
    }
};

#endif
