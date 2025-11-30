#ifndef SNAKE_CORE_SNAKE_H_
#define SNAKE_CORE_SNAKE_H_

#include "types.hpp"

namespace SnakeCore {

// Fixed-size snake body for embedded systems (no dynamic allocation)
// For terminal version, we can use a larger size or dynamic allocation
#ifndef SNAKE_MAX_SIZE
#define SNAKE_MAX_SIZE 256
#endif

class Snake {
private:
    Point body[SNAKE_MAX_SIZE];
    uint16_t headIndex;
    uint16_t tailIndex;
    uint16_t size;
    int8_t direction;
    int8_t disabledDirection;

public:
    Snake() : headIndex(0), tailIndex(0), size(0), 
              direction(DIR_RIGHT), disabledDirection(DIR_LEFT) {}
    
    void init(const Point& startPos, uint8_t initialSize, int8_t initialDir) {
        size = initialSize;
        direction = initialDir;
        headIndex = initialSize - 1;
        tailIndex = 0;
        
        // Initialize snake body
        for (uint8_t i = 0; i < initialSize; i++) {
            body[i].x = startPos.x;
            body[i].y = startPos.y - (initialSize - 1 - i);
        }
        
        updateDisabledDirection();
    }
    
    void updateDisabledDirection() {
        switch (direction) {
            case DIR_UP:    disabledDirection = DIR_DOWN;  break;
            case DIR_DOWN:  disabledDirection = DIR_UP;    break;
            case DIR_LEFT:  disabledDirection = DIR_RIGHT; break;
            case DIR_RIGHT: disabledDirection = DIR_LEFT;  break;
        }
    }
    
    bool setDirection(int8_t newDir) {
        if (newDir == DIR_NONE || newDir == disabledDirection) {
            return false;
        }
        if (newDir < DIR_DOWN || newDir > DIR_RIGHT) {
            return false;
        }
        direction = newDir;
        updateDisabledDirection();
        return true;
    }
    
    Point calculateNextHead() const {
        Point newHead = body[headIndex];
        switch (direction) {
            case DIR_UP:    newHead.x--; break;
            case DIR_DOWN:  newHead.x++; break;
            case DIR_LEFT:  newHead.y--; break;
            case DIR_RIGHT: newHead.y++; break;
        }
        return newHead;
    }
    
    void moveHead(const Point& newHead) {
        headIndex = (headIndex + 1) % SNAKE_MAX_SIZE;
        body[headIndex] = newHead;
        size++;
    }
    
    void removeTail() {
        if (size > 0) {
            tailIndex = (tailIndex + 1) % SNAKE_MAX_SIZE;
            size--;
        }
    }
    
    void move() {
        Point newHead = calculateNextHead();
        moveHead(newHead);
        removeTail();
    }
    
    void grow() {
        Point newHead = calculateNextHead();
        moveHead(newHead);
        // Don't remove tail - snake grows
    }
    
    bool collidesWithSelf(const Point& pos) const {
        // Check if position collides with any body segment (except head)
        uint16_t idx = tailIndex;
        for (uint16_t i = 0; i < size - 1; i++) {
            if (body[idx] == pos) {
                return true;
            }
            idx = (idx + 1) % SNAKE_MAX_SIZE;
        }
        return false;
    }
    
    bool containsPoint(const Point& pos) const {
        uint16_t idx = tailIndex;
        for (uint16_t i = 0; i < size; i++) {
            if (body[idx] == pos) {
                return true;
            }
            idx = (idx + 1) % SNAKE_MAX_SIZE;
        }
        return false;
    }
    
    Point getHead() const { return body[headIndex]; }
    Point getTail() const { return body[tailIndex]; }
    uint16_t getSize() const { return size; }
    int8_t getDirection() const { return direction; }
    
    // Iterator for rendering
    Point getBodySegment(uint16_t index) const {
        if (index >= size) return Point(-1, -1);
        uint16_t idx = (tailIndex + index) % SNAKE_MAX_SIZE;
        return body[idx];
    }
};

} // namespace SnakeCore

#endif
