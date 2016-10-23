#ifndef FOOD_H_
#define FOOD_H_

#include <iostream>
#include <random>

using namespace std;

class Food : public Point {

  random_device rd;

public:

  void getFood() {
    
    mt19937 rng(rd());
    uniform_int_distribution<int> xValue(2, LINES-2);
    uniform_int_distribution<int> yValue(1, COLS-2);
    
    do {
    
      auto x = xValue(rng);
      auto y = yValue(rng);

      this->x = x;
      this->y = y;

    } while((mvinch(x, y) & A_CHARTEXT) == '@');
  }

};

#endif