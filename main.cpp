#include <iostream>
#include <csignal>
#include <unistd.h>
#include "body.h"

const unsigned int twenty_milliseconds = 50000; // 50 ms

volatile sig_atomic_t interruptFlag = 0; // catch Ctrl + C event

void interruptFunction(int sig){
  
	interruptFlag = 1;	// set flag
	endwin();			// exit NCurses
}

void initializeNCurses(){

	initscr();				// Start curses mode
	//WINDOW win = newwin(COLS,LINES,0,0);
	start_color();			// Start the color functionality
	cbreak();				// Line buffering disabled
	use_default_colors();	// Use background color default
	timeout(-1);
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
}

int main()
{
	initializeNCurses();
	Body *body = new Body();
	signal(SIGINT, interruptFunction); 

	char key_stroke = DOWN;
	char disable_move = UP;

	while(!interruptFlag && !(body->gameOver)) {
	
		body->print();
	
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
		usleep(twenty_milliseconds);
	}

	endwin();
	delete body;
}

/*int main(int argc, char *argv[]) {
	int parent_x, parent_y, new_x, new_y; int score_size = 3;
	// ...
	draw_borders(field);
	draw_borders(score);
	while(1) { 
		getmaxyx(stdscr, new_y, new_x); 
		if (new_y != parent_y || new_x != parent_x) { 
			parent_x = new_x; parent_y = new_y; 
			wresize(field, new_y - score_size, new_x);
			wresize(score, score_size, new_x);
			mvwin(score, new_y - score_size, 0);
			wclear(stdscr);
			wclear(field);
			wclear(score); 
			draw_borders(field);
			draw_borders(score); 
		} // draw to our windows
		mvwprintw(field, 1, 1, "Field");
		mvwprintw(score, 1, 1, "Score");
		// refresh each window
		wrefresh(field);
		wrefresh(score);
	} // ...
}*/