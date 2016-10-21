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

	Body() {

		b = new list<Point>(); // Create a empty list of Points
		
		tam = 0; // Snake don't have size yet

		// Start snake with head position (3,1)
		this->grow(Point(1,1));
		this->grow(Point(2,1));
		this->grow(Point(3,1));

		food.setX(10);
		food.setY(10);
		mvprintw(10,10,"f");

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
		mvprintw(0, 0, intToString(tam));
		mvprintw(0, 5, s.c_str());
		mvprintw(0, 35, intToString(COLS));
		mvprintw(0, 30, intToString(LINES));
  		
  		refresh();
	}

	void print_message(const Point &p, char const *str) {

		mvprintw(p.getX(), p.getY(), str);
		refresh();
	}

	void move(int direction) {
		
		Point head = b->front();
		Point tail = b->back();

		switch (direction) {
			case UP: 	b->push_front(Point(head.getX() - 1, head.getY())); break;
			case DOWN: 	b->push_front(Point(head.getX() + 1, head.getY())); break;
			case RIGHT: b->push_front(Point(head.getX(), head.getY() + 1)); break;
			case LEFT: 	b->push_front(Point(head.getX(), head.getY() - 1)); break;
		}

		Point newHead = b->front();
		mvprintw(newHead.getX(), newHead.getY(), "@");
		mvprintw(tail.getX(), tail.getY(), " ");
		
		if(newHead != food){
			b->pop_back();
		} else {
			tam++;
		}
	}

};

#endif