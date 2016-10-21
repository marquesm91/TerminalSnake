#ifndef BODY_H_
#define BODY_H_

#include <list>
#include <string>
#include <ncurses.h>
#include "point.h"

// Integer numbers for Arrow Keys and Movements
#define UP 3
#define DOWN 2
#define RIGHT 5		
#define LEFT 4

// Use default color when init_curses
#define COLOR_DEFAULT -1

using namespace std;

class Body{

	list<Point> *b;
	unsigned tam;
	Point food;

public:

	bool gameOver;

	Body() {

		b = new list<Point>(); // Create a empty list of Points
		
		tam = 0; // Snake don't have size yet

		gameOver = false;
		// Start snake with head position (3,1)
		this->grow(Point(1,1));
		this->grow(Point(2,1));
		this->grow(Point(3,1));
		this->grow(Point(4,1));
		this->grow(Point(5,1));
		this->grow(Point(6,1));
		this->grow(Point(7,1));
		this->grow(Point(8,1));

		food.getFood();
		mvprintw(food.getX(),food.getY(),"f");
		//hashtable.emplace(to_string(food.getX() + food.getY()), "f");

	}

	~Body() {

		if(!b->empty())
			b->clear();
		delete b;
	}

	void grow(const Point &p) {
	
		b->push_front(p);
		tam++;
	}

	const char * intToString(int n) {

		return to_string(n).c_str();
	}

	void print() {

		Point head = b->front();

		string s = "(" + to_string(head.getX());
		s += " ,";
		s += to_string(head.getY());
		s += ")";

		string a = "(" + to_string(food.getX());
		a += " ,";
		a += to_string(food.getY());
		a += ")";

		mvprintw(0, 0, intToString(tam));
		mvprintw(0, 5, s.c_str());
		mvprintw(0, 35, intToString(COLS));
		mvprintw(0, 30, intToString(LINES));

		mvprintw(0, 56, a.c_str());
  		
  	refresh();
	}

	void print_message(const Point &p, char const *str) {

		mvprintw(p.getX(), p.getY(), str);
		refresh();
	}

	void move(int direction) {
		
		Point head = b->front();
		Point tail = b->back();
		Point newHead;

		switch (direction) {
			case UP: 		newHead.setX(head.getX() - 1); newHead.setY(head.getY()); break;
			case DOWN: 	newHead.setX(head.getX() + 1); newHead.setY(head.getY()); break; 
			case RIGHT: newHead.setX(head.getX()); newHead.setY(head.getY() + 1); break;
			case LEFT: 	newHead.setX(head.getX()); newHead.setY(head.getY() - 1); break;
		}	
		
		if (tryToMove(newHead)) {
			b->push_front(newHead);
			mvprintw(newHead.getX(), newHead.getY(), "@");
		} else {
			this->gameOver = true;
			return;
		}
		
		if(newHead != food){
			b->pop_back();
		} else {
			tam++;
			food.getFood();
			mvprintw(food.getX(),food.getY(),"f");
		}

		mvprintw(tail.getX(), tail.getY(), " ");
	}

	bool tryToMove(Point head) {

		char ch = mvinch(head.getX(), head.getY()) & A_CHARTEXT;
		return ch != '@';
	}

};

#endif