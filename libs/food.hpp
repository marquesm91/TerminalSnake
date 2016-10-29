#ifndef FOOD_H_
#define FOOD_H_

#include <random>

using namespace std;

class Food : public Point {

private:
  random_device rd;

public:

  Food() {}
  ~Food() {}

  void getFood() {
    
    mt19937 rng(rd());

    // LINES and COLS are set by ncurses when called initscr();
    uniform_int_distribution<int> xValue(2, LINES-2);
    uniform_int_distribution<int> yValue(1, COLS-2);
    
    auto x = xValue(rng);
    auto y = yValue(rng);

    this->x = x;
    this->y = y;
  }
};

#endif