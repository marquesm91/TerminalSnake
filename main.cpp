#include <csignal>
#include "./libs/terminal.hpp"
#include <iostream>
#include "./libs/game.hpp"
#include "./libs/menu.hpp"
#include "./libs/highscore.hpp"

void setupGame() {
    initscr();              // Initialize terminal
    start_color();          // Start color support
    cbreak();               // Line buffering disabled
    use_default_colors();   // Use default terminal colors
    curs_set(0);            // Hide cursor
    keypad(stdscr, TRUE);   // Enable arrow keys
    noecho();               // Disable input echo
    nodelay(stdscr, TRUE);  // Non-blocking input
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
            // Clean terminal completely before next game
            full_clear_screen();
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
                full_clear_screen();       // Restart terminal for game
                nodelay(stdscr, TRUE);     // Disable blocking for game
                
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
    setupGame();
    signal(SIGINT, interruptFunction);

    showMenu();

    endwin();
    return 0;
}