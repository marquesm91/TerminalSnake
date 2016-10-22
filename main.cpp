#include <iostream>
#include <csignal>
#include <unistd.h>
#include "body.h"

volatile sig_atomic_t interruptFlag = 0; // catch Ctrl + C event

void interruptFunction(int sig) {
  
	interruptFlag = 1;	// set flag
	endwin();			// exit NCurses
}

void initializeGame() {

	initscr();				// Start curses mode
	//WINDOW win = newwin(COLS,LINES,0,0);
	start_color();			// Start the color functionality
	cbreak();				// Line buffering disabled
	use_default_colors();	// Use background color default
	//timeout(5000);
	curs_set(0);			// hide cursor console
	keypad(stdscr, TRUE);	// For Arrow Keys
	noecho();				// Disable echo() in getch()
	nodelay(stdscr, TRUE);	// Remove the getch() delay and use my own.	

	// Set up Colors
	init_pair(1, COLOR_WHITE, COLOR_DEFAULT); 	// 0 and 128(bold)
	init_pair(2, COLOR_CYAN, COLOR_DEFAULT); 	// 2 and 256(bold)
	init_pair(3, COLOR_YELLOW, COLOR_DEFAULT); 	// 4 and 512(bold)
 	init_pair(4, COLOR_GREEN, COLOR_DEFAULT); 	// 8 and 1024(bold)
	init_pair(5, COLOR_MAGENTA, COLOR_DEFAULT); // 16 and 2048(bold)
	init_pair(6, COLOR_RED, COLOR_DEFAULT); 	// 32 and 4096(bold)
	init_pair(7, COLOR_BLUE, COLOR_DEFAULT); 	// 64 and 8192(bold)

	// Set up board
	mvprintw(0,0, "SCORE: ");
	mvprintw(0, 13, "SIZE: ");
	mvprintw(0, 25, "H.POS: (");
	mvprintw(0, 35, ",");
	mvprintw(0, 38, ")  F.POS: (");
	mvprintw(0, 51, ",");
	mvprintw(0, 54, ")");
	mvprintw(0, 60, "HIGHSCORE: ");

	for(int i = 1; i < LINES - 1; i++){
		mvprintw(i, 0, "|"); mvprintw(i, COLS - 1, "|");	
	}

	for(int j = 1; j < COLS - 1; j++){
		mvprintw(1, j, "-"); mvprintw(LINES - 1, j, "-");
	}

	mvprintw(1,0,"+");
	mvprintw(1,COLS - 1,"+");
	mvprintw(LINES - 1,0,"+");
	mvprintw(LINES - 1, COLS - 1,"+");
	refresh();
}

int main()
{
	initializeGame();
	Body *body = new Body();
	signal(SIGINT, interruptFunction); 

	char key_stroke = RIGHT;
	char disable_move = LEFT;

	while(!interruptFlag && !(body->gameOver)) {
	
		char aux = getch();

		if(aux != ERR && aux != disable_move)
			key_stroke = (aux >= 2 && aux <= 5) ? aux : key_stroke;

		switch ((int)key_stroke) {
			case UP: 	body->move(UP); 	disable_move = DOWN; 	break;
			case DOWN: 	body->move(DOWN); 	disable_move = UP; 		break;
			case RIGHT: body->move(RIGHT);	disable_move = LEFT; 	break;
			case LEFT: 	body->move(LEFT);	disable_move = RIGHT;	break;
		}

		//ungetch(aux);
		usleep(delay);
	}

	endwin();
	delete body;
}
