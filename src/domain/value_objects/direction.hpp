/**
 * @file direction.hpp
 * @brief Direction Value Object - Represents movement directions
 * 
 * Clean Architecture: Domain Layer - Value Object
 */

#ifndef DOMAIN_VALUE_OBJECTS_DIRECTION_HPP
#define DOMAIN_VALUE_OBJECTS_DIRECTION_HPP

#include <cstdint>
#include "domain/value_objects/point.hpp"

namespace Snake {
namespace Domain {

enum class Direction : uint8_t {
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
    None = 255
};

/**
 * @brief Utilitários para Direction
 */
class DirectionUtils {
public:
    // Check if two directions are opposite
    static bool areOpposite(Direction a, Direction b) {
        if (a == Direction::None || b == Direction::None) return false;
        return (a == Direction::Up && b == Direction::Down) ||
               (a == Direction::Down && b == Direction::Up) ||
               (a == Direction::Left && b == Direction::Right) ||
               (a == Direction::Right && b == Direction::Left);
    }
    
    // Convert direction to movement delta
    static Point toDelta(Direction dir) {
        switch (dir) {
            case Direction::Up:    return Point(0, -1);
            case Direction::Down:  return Point(0, 1);
            case Direction::Left:  return Point(-1, 0);
            case Direction::Right: return Point(1, 0);
            default:               return Point(0, 0);
        }
    }
    
    // Convert ncurses key to direction
    static Direction fromKey(int key) {
        switch (key) {
            case 3:  return Direction::Up;    // KEY_UP
            case 2:  return Direction::Down;  // KEY_DOWN
            case 4:  return Direction::Left;  // KEY_LEFT
            case 5:  return Direction::Right; // KEY_RIGHT
            default: return Direction::None;
        }
    }
    
    // Direção oposta
    static Direction opposite(Direction dir) {
        switch (dir) {
            case Direction::Up:    return Direction::Down;
            case Direction::Down:  return Direction::Up;
            case Direction::Left:  return Direction::Right;
            case Direction::Right: return Direction::Left;
            default:               return Direction::None;
        }
    }
    
    // Convert to string (debug)
    static const char* toString(Direction dir) {
        switch (dir) {
            case Direction::Up:    return "Up";
            case Direction::Down:  return "Down";
            case Direction::Left:  return "Left";
            case Direction::Right: return "Right";
            default:               return "None";
        }
    }
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_VALUE_OBJECTS_DIRECTION_HPP
