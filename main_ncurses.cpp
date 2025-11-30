#include <csignal>
#include <ncurses.h>
#include <iostream>
#include "./libs/game.hpp"
#include "./libs/menu.hpp"
#include "./libs/highscore.hpp"

void initNCurses() {

    initscr();              // Start curses mode
    start_color();          // Start the color functionality
    cbreak();               // Line buffering disabled
    use_default_colors();   // Use background color default
    curs_set(0);            // hide cursor console
    keypad(stdscr, TRUE);   // For Arrow Keys
    noecho();               // Disable echo() in getch()
    nodelay(stdscr, TRUE);  // Remove the getch() delay and use my own. 

    // Modern color scheme
    init_pair(1, COLOR_CYAN, COLOR_DEFAULT);      // Border color
    init_pair(2, COLOR_GREEN, COLOR_DEFAULT);     // Snake head
    init_pair(3, COLOR_GREEN, COLOR_DEFAULT);     // Snake body
    init_pair(4, COLOR_RED, COLOR_DEFAULT);       // Food
    init_pair(5, COLOR_YELLOW, COLOR_DEFAULT);    // Score
    init_pair(6, COLOR_MAGENTA, COLOR_DEFAULT);   // Highscore
    init_pair(7, COLOR_RED, COLOR_DEFAULT);       // Game over
    init_pair(8, COLOR_WHITE, COLOR_BLUE);        // Status bar background

}

volatile sig_atomic_t interruptFlag = 0; // catch Ctrl + C event

void interruptFunction(int /* sig */) {
    interruptFlag = 1;  // set flag
    endwin();           // exit NCurses
}

bool runGame(int level) {

    char ch;
    Game *g = new Game(level);
    
    while(!interruptFlag && !g->isGameOver());
    
    bool playAgain = false;
    
    if (!interruptFlag) {

        do{

            ch = getch();
            ch = toupper(ch);

        } while (ch != 'Y' && ch != 'N' && ch != '\n' && !interruptFlag);

        if (ch == 'Y' || ch == '\n'){
            playAgain = true;
        }
    }
    
    delete g;
    return playAgain;
}

void showMenu() {
    Menu menu;
    Highscore highscore;
    
    nodelay(stdscr, FALSE);  // Enable blocking for menu navigation
    
    bool running = true;
    while (running && !interruptFlag) {
        int choice = menu.showMainMenu(highscore.get());
        
        switch (choice) {
            case 0: // Start Game
                clear();
                nodelay(stdscr, TRUE);  // Disable blocking for game
                
                while (runGame(menu.getDifficultyLevel())) {
                    // Reload highscore for next game
                    highscore = Highscore();
                }
                
                nodelay(stdscr, FALSE);  // Re-enable blocking for menu
                // Reload highscore after game
                highscore = Highscore();
                break;
                
            case 1: // Settings
                while (!menu.showSettings() && !interruptFlag) {
                    // Stay in settings until user presses 'q'
                }
                break;
                
            case 2: // Exit
                running = false;
                break;
                
            default:
                // Continue in menu
                break;
        }
    }
}

int main()
{
    initNCurses();
    signal(SIGINT, interruptFunction);

    showMenu();

    endwin();
    return 0;
}