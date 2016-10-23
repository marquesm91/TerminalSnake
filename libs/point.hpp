#ifndef POINT_H_
#define POINT_H_

#include <iostream>

using namespace std;

class Point{

protected:
  
  int x;
  int y;

public:

  Point() { x = 0; y = 0; }
  Point(int x, int y) { this->x = x; this->y = y; }
  Point(const Point &obj) { x = obj.getX(); y = obj.getY(); };
  
  bool operator!=(const Point& p) { 
    return (x != p.getX() || y != p.getY()) ? true : false;
  }

  bool operator==(const Point p) { 
    return (x == p.getX() && y == p.getY()) ? true : false;
  }
  
  int getX() const { return x; }
  int getY() const { return y; }
  void setX(int x) { this->x = x; };
  void setY(int y) { this->y = y; };

};

#endif