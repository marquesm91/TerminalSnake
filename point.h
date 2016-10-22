#ifndef POINT_H_
#define POINT_H_

#include <iostream>
#include <random>

using namespace std;

class Point{

  int x;
  int y;
  random_device rd;     // only used once to initialise (seed) engine

public:

  Point() { x = 0; y = 0; }
  Point(int _x, int _y) { this->x = _x; this->y = _y; }
  Point(const Point &obj) { this->x = obj.getX(); this->y = obj.getY(); };
  
  bool operator!=(const Point& p) { 
    return (this->x != p.getX() || this->y != p.getY()) ? true : false;
  }

  bool operator==(const Point p) { 
    return (x == p.getX() && y == p.getY()) ? true : false;
  }
  
  int getX() const { return x; }
  int getY() const { return y; }
  void setX(int _x) { this->x = _x; };
  void setY(int _y) { this->y = _y; };

  void getFood() {
    
    mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    uniform_int_distribution<int> xValue(2,LINES-2); // guaranteed unbiased
    uniform_int_distribution<int> yValue(1,COLS-2); // guaranteed unbiased

    auto x = xValue(rng);
    auto y = yValue(rng);

    this->x = x;
    this->y = y;
    //return Point(randomX, randomY);
  }

};

#endif