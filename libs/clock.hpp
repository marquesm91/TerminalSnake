#ifndef CLOCK_H_
#define CLOCK_H_

#include <chrono>

using namespace std::chrono;

class Clock {

private:
  
  steady_clock::time_point now;
  steady_clock::time_point last;

public:

  Clock() {
    now = steady_clock::now();
    last = now;
  }

  double getTimestamp() {
    now = steady_clock::now();
    return duration_cast<milliseconds>(now - last).count();
  }

  void reset() {
    last = now;
  }
};

#endif