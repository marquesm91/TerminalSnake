#ifndef POINT_H_
#define POINT_H_

#include <iostream>

class Point{

  int x;
  int y;

public:

  Point() { x = 0; y = 0; }
  Point(int _x, int _y) { this->x = _x; this->y = _y; }
  Point(const Point &obj) { this->x = obj.getX(); this->y = obj.getY(); };
  
  bool operator!=(const Point& p) { 
    return (this->x != p.getX() || this->y != p.getY()) ? true : false;
  }

  bool operator==(const Point& p) { 
    return (this->x == p.getX() && this->y == p.getY()) ? true : false;
  }
  
  int getX() const { return x; }
  int getY() const { return y; }
  void setX(int _x) { this->x = _x; };
  void setY(int _y) { this->y = _y; };

};

#endif