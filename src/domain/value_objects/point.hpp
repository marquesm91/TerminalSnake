/**
 * @file point.hpp
 * @brief Point Value Object - Immutable, represents 2D coordinates
 * 
 * Clean Architecture: Domain Layer - Value Object
 * - Immutable after creation
 * - Comparison by value
 * - No side effects
 */

#ifndef DOMAIN_VALUE_OBJECTS_POINT_HPP
#define DOMAIN_VALUE_OBJECTS_POINT_HPP

#include <cstdint>
#include <functional>

namespace Snake {
namespace Domain {

class Point {
public:
    // Construtores
    constexpr Point() : x_(0), y_(0) {}
    constexpr Point(int16_t x, int16_t y) : x_(x), y_(y) {}
    
    // Getters (sem setters - imutável)
    constexpr int16_t x() const { return x_; }
    constexpr int16_t y() const { return y_; }
    
    // Operations that return new Points
    constexpr Point moved(int16_t dx, int16_t dy) const {
        return Point(x_ + dx, y_ + dy);
    }
    
    constexpr Point up() const { return moved(0, -1); }
    constexpr Point down() const { return moved(0, 1); }
    constexpr Point left() const { return moved(-1, 0); }
    constexpr Point right() const { return moved(1, 0); }
    
    // Operadores de comparação
    constexpr bool operator==(const Point& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }
    
    constexpr bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    // Para uso em containers ordenados
    bool operator<(const Point& other) const {
        if (x_ != other.x_) return x_ < other.x_;
        return y_ < other.y_;
    }
    
    // Distância Manhattan
    int16_t manhattanDistance(const Point& other) const {
        int16_t dx = x_ > other.x_ ? x_ - other.x_ : other.x_ - x_;
        int16_t dy = y_ > other.y_ ? y_ - other.y_ : other.y_ - y_;
        return dx + dy;
    }
    
    // Check if within bounds
    constexpr bool isWithinBounds(int16_t width, int16_t height) const {
        return x_ >= 0 && x_ < width && y_ >= 0 && y_ < height;
    }

private:
    int16_t x_;
    int16_t y_;
};

} // namespace Domain
} // namespace Snake

// Hash function para uso em unordered_map/set
namespace std {
template<>
struct hash<Snake::Domain::Point> {
    size_t operator()(const Snake::Domain::Point& p) const {
        return hash<int32_t>()(p.x() << 16 | (p.y() & 0xFFFF));
    }
};
}

#endif // DOMAIN_VALUE_OBJECTS_POINT_HPP
