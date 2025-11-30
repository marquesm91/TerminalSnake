/**
 * @file snake.hpp
 * @brief Snake Entity - Main domain entity
 * 
 * Clean Architecture: Domain Layer - Entity
 * - Has identity (head position)
 * - Contains business logic
 * - Infrastructure independent
 */

#ifndef DOMAIN_ENTITIES_SNAKE_HPP
#define DOMAIN_ENTITIES_SNAKE_HPP

#include <cstdint>
#include <array>
#include "domain/value_objects/point.hpp"
#include "domain/value_objects/direction.hpp"

namespace Snake {
namespace Domain {

/**
 * @brief Snake Entity - Manages the snake body
 * 
 * Uses fixed array (no dynamic allocation) to support Arduino.
 * Body is stored as a list where body_[0] is always the head
 * and body_[size_-1] is always the tail.
 */
class SnakeEntity {
public:
    static constexpr uint16_t MAX_SIZE = 256;
    
    SnakeEntity() : size_(0), direction_(Direction::Right) {}
    
    /**
     * @brief Initialize the snake with position and initial size
     */
    void initialize(Point headPos, uint16_t initialSize, Direction dir) {
        size_ = initialSize;
        direction_ = dir;
        
        // Create body behind head
        Point delta = DirectionUtils::toDelta(DirectionUtils::opposite(dir));
        for (uint16_t i = 0; i < initialSize && i < MAX_SIZE; ++i) {
            body_[i] = Point(
                headPos.x() + delta.x() * static_cast<int16_t>(i),
                headPos.y() + delta.y() * static_cast<int16_t>(i)
            );
        }
    }
    
    /**
     * @brief Move the snake in the current direction
     * @param grow If true, increases size (doesn't remove tail)
     * @return New head position
     */
    Point move(bool grow = false) {
        if (size_ == 0) return Point();
        
        // Calculate new head position
        Point currentHead = body_[0];
        Point delta = DirectionUtils::toDelta(direction_);
        Point newHead = currentHead.moved(delta.x(), delta.y());
        
        if (grow && size_ < MAX_SIZE) {
            // Grow: shift all segments back and insert new head
            for (uint16_t i = size_; i > 0; --i) {
                body_[i] = body_[i - 1];
            }
            body_[0] = newHead;
            ++size_;
        } else {
            // Move without growing: shift all segments back (discarding tail)
            for (uint16_t i = size_ - 1; i > 0; --i) {
                body_[i] = body_[i - 1];
            }
            body_[0] = newHead;
        }
        
        return newHead;
    }
    
    /**
     * @brief Try to change direction
     * @return true if direction was changed
     */
    bool setDirection(Direction newDir) {
        if (newDir == Direction::None) return false;
        if (DirectionUtils::areOpposite(direction_, newDir)) return false;
        
        direction_ = newDir;
        return true;
    }
    
    /**
     * @brief Check if the snake collided with itself
     */
    bool hasSelfCollision() const {
        if (size_ <= 1) return false;
        
        Point h = head();
        for (uint16_t i = 1; i < size_; ++i) {
            if (segmentAt(i) == h) return true;
        }
        return false;
    }
    
    /**
     * @brief Check if a point is occupied by the body
     */
    bool occupies(Point p) const {
        for (uint16_t i = 0; i < size_; ++i) {
            if (segmentAt(i) == p) return true;
        }
        return false;
    }
    
    /**
     * @brief Check if the head is at a specific point
     */
    bool headAt(Point p) const {
        return head() == p;
    }
    
    // Getters
    Point head() const {
        return size_ > 0 ? body_[0] : Point();
    }
    
    Point tail() const {
        if (size_ == 0) return Point();
        return body_[size_ - 1];
    }
    
    Point segmentAt(uint16_t index) const {
        if (index >= size_) return Point();
        return body_[index];
    }
    
    uint16_t size() const { return size_; }
    Direction direction() const { return direction_; }
    
    // Iterador para acesso ao corpo (read-only)
    class Iterator {
    public:
        Iterator(const SnakeEntity* snake, uint16_t index)
            : snake_(snake), index_(index) {}
        
        Point operator*() const { return snake_->segmentAt(index_); }
        Iterator& operator++() { ++index_; return *this; }
        bool operator!=(const Iterator& other) const { 
            return index_ != other.index_; 
        }
        
    private:
        const SnakeEntity* snake_;
        uint16_t index_;
    };
    
    Iterator begin() const { return Iterator(this, 0); }
    Iterator end() const { return Iterator(this, size_); }

private:
    std::array<Point, MAX_SIZE> body_;
    uint16_t size_;
    Direction direction_;
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_ENTITIES_SNAKE_HPP
