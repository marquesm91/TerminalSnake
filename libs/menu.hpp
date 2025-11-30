#ifndef MENU_H_
#define MENU_H_

#include <string>
#include <vector>
#include "common.hpp"
#include "terminal.hpp"

class Menu {

private:
    int selectedOption;
    int difficultyLevel;
    int animationFrame;
    std::vector<std::string> menuOptions;
    std::vector<std::string> difficultyOptions;
    bool userSignedIn;

    void drawLogo(int startY, int startX) {
        std::vector<std::string> logo = {
            "  _______                  _             _   _____             _",
            " |__   __|                (_)           | | / ____|           | |",
            "    | | ___ _ __ _ __ ___  _ _ __   __ _| || (___  _ __   __ _| | _____ ",
            "    | |/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` | | \\___ \\| '_ \\ / _` | |/ / _ \\",
            "    | |  __/ |  | | | | | | | | | | (_| | | ____) | | | | (_| |   <  __/",
            "    |_|\\___|_|  |_| |_| |_|_|_| |_|\\__,_|_||_____/|_| |_|\\__,_|_|\\_\\___|"
        };

        int logoWidth = logo[0].length();
        int logoStartX = startX - (logoWidth / 2);
        
        // Ensure we don't draw off-screen
        if (logoStartX < 0) logoStartX = 0;

        // Animation: Shimmer effect moving from left to right
        int shimmerPos = (animationFrame * 2) % (logoWidth + 20); // Speed 2x, +20 for pause

        for (size_t i = 0; i < logo.size(); i++) {
            for (size_t j = 0; j < logo[i].length(); j++) {
                char c = logo[i][j];
                if (c == ' ') continue;

                // Calculate distance from shimmer position
                int dist = abs((int)j - (shimmerPos - 10)); // Offset to start off-screen
                
                if (dist < 3) {
                    attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE); // Bright Green (Shimmer center)
                } else if (dist < 6) {
                    attron(COLOR_PAIR(2) | A_BOLD);             // Green Bold (Shimmer edge)
                } else {
                    attron(COLOR_PAIR(2));                      // Normal Green
                }
                
                mvaddch(startY + i, logoStartX + j, c);
                
                attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            }
        }

        // Subtitle with pulsing effect
        if ((animationFrame / 5) % 2 == 0) {
            attron(COLOR_PAIR(5) | A_BOLD);
        } else {
            attron(COLOR_PAIR(5));
        }
        mvprintw(startY + logo.size() + 1, startX - 10, "Terminal Edition v2.0");
        attroff(COLOR_PAIR(5) | A_BOLD);
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
