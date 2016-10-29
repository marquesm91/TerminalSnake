#ifndef BODY_H_
#define BODY_H_

#include <list>
#include "point.hpp"
#include "common.hpp"

using namespace std;

class Body{

private:

	list<Point> *body;
	int direction;
	int disableDirection;

public:

	Body() {

		body = new list<Point>(); // Create a empty list of Points

    	body->push_front(Point(5,5));
    	body->push_front(Point(5,6));
    	body->push_front(Point(5,7)); // Head start position

    	this->validateDirection(RIGHT); // Starting moving to the right
	}

	~Body() {

		if(!body->empty())
			body->clear();
		delete body;
	}

	void validateDirection(int direction) {
		if (direction != ERR && direction != disableDirection && direction >= 2 && direction <= 5) {
			this->direction = direction;

			// Set disable direction
			switch (this->direction) {
				case UP: disableDirection = DOWN; break;
				case DOWN: disableDirection = UP; break;
				case LEFT: disableDirection = RIGHT; break;
				case RIGHT: disableDirection = LEFT; break;
			}
		}
	}

	Point investigatePosition(){
		Point newHead;

		switch (this->direction) {
			case UP: newHead.setX(this->getHead().getX() - 1);newHead.setY(this->getHead().getY()); break;
			case DOWN: newHead.setX(this->getHead().getX() + 1);newHead.setY(this->getHead().getY()); break;
			case LEFT: newHead.setX(this->getHead().getX()); newHead.setY(this->getHead().getY() - 1); break;
			case RIGHT:	newHead.setX(this->getHead().getX()); newHead.setY(this->getHead().getY() + 1); break;
		}

		return newHead;
	}
	
	int getDirection() const { return direction; }

	int getDisableDirection() const { return disableDirection; }

	Point getHead() const { return body->front(); }
	void setHead(const Point &p) { body->push_front(p); }

	Point getTail() const { return body->back(); }
	void removeTail() { body->pop_back(); }
	
	int getSize() const { return body->size(); }

};

#endif