#ifndef BODY_H_
#define BODY_H_

#include <list>
#include <string>
#include <ncurses.h>
#include "point.hpp"
#include "food.hpp"

// Integer numbers for Arrow Keys and Movements
#define UP 3
#define DOWN 2
#define RIGHT 5		
#define LEFT 4

// Use default color when init_curses
#define COLOR_DEFAULT -1

const int DELAY = 35; // 35 ms
const int initialSnakeSize = 3;

using namespace std;

class Body{

	list<Point> *b;
	Food *food;
	unsigned tam;

public:

	bool gameOver;

	Body() {

		b = new list<Point>(); // Create a empty list of Points
		
		food = new Food();
		food->getFood();
		mvprintw(food->getX(), food->getY(),"f");

		// Start snake with head position (5,7)
		b->push_front(Point(5,5));
		b->push_front(Point(5,6));
		b->push_front(Point(5,7));

		tam = initialSnakeSize;
		gameOver = false;
		
		// print Score, Size and Food initial position
		mvprintw(0, 7, to_string((tam - initialSnakeSize) * DELAY).c_str());
		mvprintw(0, 19, to_string(tam).c_str());
		string x = food->getX() >= 10 ? to_string(food->getX()) : to_string(food->getX()) + " ";
		string y = food->getY() >= 10 ? to_string(food->getY()) : to_string(food->getY()) + " ";
		mvprintw(0, 49, x.c_str());
		mvprintw(0, 52, y.c_str());
		

		refresh();
	}

	~Body() {

		if(!b->empty())
			b->clear();
		delete b;
		delete food;
	}

	void updateScore() {
		
		this->tam++;

		// print Score
		mvprintw(0, 7, to_string((tam - initialSnakeSize) * DELAY).c_str());
		
		// print Size
		mvprintw(0, 19, to_string(this->tam).c_str());
		
		// Generate new Food
		food->getFood();
		mvprintw(food->getX(), food->getY(), "f");

		// print Food position
		string x = food->getX() >= 10 ? to_string(food->getX()) : to_string(food->getX()) + " ";
		string y = food->getY() >= 10 ? to_string(food->getY()) : to_string(food->getY()) + " ";
		mvprintw(0, 49, x.c_str());
		mvprintw(0, 52, y.c_str());
	}

	void move(int direction) {
		
		Point head = b->front();
		Point newHead;

		switch (direction) {
			case UP: 	newHead.setX(head.getX() - 1); newHead.setY(head.getY()); break;
			case DOWN: 	newHead.setX(head.getX() + 1); newHead.setY(head.getY()); break; 
			case RIGHT: newHead.setX(head.getX()); newHead.setY(head.getY() + 1); break;
			case LEFT: 	newHead.setX(head.getX()); newHead.setY(head.getY() - 1); break;
		}	
		
		if (findObstacle(newHead))
			gameOver = true;
		else
			refresh();
	}

	bool findObstacle(Point newHead) {

		char ch = mvinch(newHead.getX(), newHead.getY()) & A_CHARTEXT;
		Point tail = b->back();

		if (ch == 'f')
			updateScore();
		else if (ch == '@' || ch == '-' || ch == '|')
			return true;
		else {
			b->pop_back();
			mvprintw(tail.getX(), tail.getY(), " ");
		}

		b->push_front(newHead);
		mvprintw(newHead.getX(), newHead.getY(), "@");

		string x = newHead.getX() >= 10 ? to_string(newHead.getX()) : to_string(newHead.getX()) + " ";
		string y = newHead.getY() >= 10 ? to_string(newHead.getY()) : to_string(newHead.getY()) + " ";
		mvprintw(0, 33, x.c_str());
		mvprintw(0, 36, y.c_str());

		return false;
	}

};

#endif