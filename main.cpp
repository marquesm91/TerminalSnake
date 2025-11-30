#include <csignal>
#include <ncurses.h>
#include <iostream>
#include "./libs/game.hpp"
#include "./libs/menu.hpp"
#include "./libs/highscore.hpp"
#include "./libs/auth.hpp"
#include "./libs/leaderboard.hpp"
#include "./libs/anticheat.hpp"

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

bool runGame(int level, Auth* auth, Leaderboard* leaderboard) {

    char ch;
    Game *g = new Game(level);
    AntiCheat anticheat;
    anticheat.setDifficulty(level);
    
    while(!interruptFlag && !g->isGameOver()) {
        // Record inputs for anti-cheat
        // Note: In a full implementation, we'd need to hook into the game's input handling
    }
    
    bool playAgain = false;
    
    if (!interruptFlag) {
        // Get final score from game
        int finalScore = g->getScore();
        int snakeSize = g->getSnakeSize();
        
        // Update local highscore
        Highscore highscore;
        highscore.set(finalScore);
        
        // Submit to leaderboard if authenticated
        if (auth && auth->isAuthenticated() && leaderboard) {
            anticheat.setScore(finalScore);
            auto sessionData = anticheat.getSessionData();
            
            if (sessionData.confidenceScore >= 30) {
                leaderboard->submitScore(sessionData, snakeSize);
                leaderboard->showUserRank(finalScore);
            }
        }

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
    Auth auth;
    Leaderboard leaderboard(&auth);
    
    // Update menu if user is already signed in
    if (auth.isAuthenticated()) {
        menu.setUserSignedIn(true, auth.getDisplayName());
    }
    
    nodelay(stdscr, FALSE);  // Enable blocking for menu navigation
    
    bool running = true;
    while (running && !interruptFlag) {
        int choice = menu.showMainMenu(highscore.get());
        
        switch (choice) {
            case 0: // Start Game
                clear();
                nodelay(stdscr, TRUE);  // Disable blocking for game
                
                while (runGame(menu.getDifficultyLevel(), &auth, &leaderboard)) {
                    // Reload highscore for next game
                    highscore = Highscore();
                }
                
                nodelay(stdscr, FALSE);  // Re-enable blocking for menu
                // Reload highscore after game
                highscore = Highscore();
                break;
            
            case 1: // Leaderboard
                leaderboard.fetch();
                leaderboard.display();
                break;
                
            case 2: // Settings
                while (!menu.showSettings() && !interruptFlag) {
                    // Stay in settings until user presses 'q'
                }
                break;
            
            case 3: // Sign In / Sign Out
                if (auth.isAuthenticated()) {
                    auth.logout();
                    menu.setUserSignedIn(false);
                } else {
                    if (auth.authenticateWithDeviceFlow()) {
                        menu.setUserSignedIn(true, auth.getDisplayName());
                    }
                }
                break;
                
            case 4: // Exit
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