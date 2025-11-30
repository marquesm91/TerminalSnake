#ifndef SNAKE_CORE_RANDOM_H_
#define SNAKE_CORE_RANDOM_H_

#include <cstdint>

namespace SnakeCore {

// Simple deterministic PRNG for replay compatibility
// Uses xorshift32 algorithm - works identically on all platforms
class Random {
private:
    uint32_t state;

public:
    Random() : state(1) {}
    
    void seed(uint32_t s) {
        state = s ? s : 1;  // State must not be zero
    }
    
    uint32_t getSeed() const { return state; }
    
    // Generate next random number
    uint32_t next() {
        // xorshift32 algorithm
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }
    
    // Generate random number in range [min, max]
    int32_t range(int32_t min, int32_t max) {
        if (min >= max) return min;
        uint32_t range = static_cast<uint32_t>(max - min + 1);
        return min + static_cast<int32_t>(next() % range);
    }
    
    // Generate random point within bounds
    void randomPoint(int16_t& x, int16_t& y, 
                     int16_t minX, int16_t maxX,
                     int16_t minY, int16_t maxY) {
        x = static_cast<int16_t>(range(minX, maxX));
        y = static_cast<int16_t>(range(minY, maxY));
    }
};

} // namespace SnakeCore

#endif
