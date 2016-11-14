#include <csignal>
#include <ncurses.h>
#include <iostream>
#include "./libs/game.hpp"

void initNCurses() {

    initscr();              // Start curses mode
    //WINDOW win = newwin(COLS,LINES,0,0);
    start_color();          // Start the color functionality
    cbreak();               // Line buffering disabled
    use_default_colors();   // Use background color default
    //timeout(100);
    curs_set(0);            // hide cursor console
    keypad(stdscr, TRUE);   // For Arrow Keys
    noecho();               // Disable echo() in getch()
    nodelay(stdscr, TRUE);  // Remove the getch() delay and use my own. 

    // Set up Colors
    init_pair(1, COLOR_WHITE, COLOR_DEFAULT);   // 0 and 128(bold)
    init_pair(2, COLOR_CYAN, COLOR_DEFAULT);    // 2 and 256(bold)
    init_pair(3, COLOR_YELLOW, COLOR_DEFAULT);  // 4 and 512(bold)
    init_pair(4, COLOR_GREEN, COLOR_DEFAULT);   // 8 and 1024(bold)
    init_pair(5, COLOR_MAGENTA, COLOR_DEFAULT); // 16 and 2048(bold)
    init_pair(6, COLOR_RED, COLOR_DEFAULT);     // 32 and 4096(bold)
    init_pair(7, COLOR_BLUE, COLOR_DEFAULT);    // 64 and 8192(bold)

}

volatile sig_atomic_t interruptFlag = 0; // catch Ctrl + C event

void interruptFunction(int sig) {
  
    interruptFlag = 1;  // set flag
    endwin();           // exit NCurses
}

void initGame(Game &g) {

    char ch;
    
    while(!interruptFlag && !g.isGameOver());
    
    if (!interruptFlag) {

        mvprintw(18,4,"Try again? (Y/n)");

        do{

            ch = getch();
            ch = toupper(ch);

        } while (ch != 'Y' && ch != 'N' && ch != '\n' && !interruptFlag);

        if (ch == 'Y' || ch == '\n'){
            g.reset();
            initGame(g);
        }
    }
}

int main()
{
    initNCurses();
	signal(SIGINT, interruptFunction);

    Game *g = new Game(9);
    
    initGame(*g);

    delete g;
    
}