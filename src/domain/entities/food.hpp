/**
 * @file food.hpp
 * @brief Food Entity - Represents food in the game
 * 
 * Clean Architecture: Domain Layer - Entity
 */

#ifndef DOMAIN_ENTITIES_FOOD_HPP
#define DOMAIN_ENTITIES_FOOD_HPP

#include "domain/value_objects/point.hpp"

namespace Snake {
namespace Domain {

/**
 * @brief Food Entity - Food position on the board
 */
class FoodEntity {
public:
    FoodEntity() : position_(Point()), active_(false) {}
    explicit FoodEntity(Point pos) : position_(pos), active_(true) {}
    
    Point position() const { return position_; }
    bool isActive() const { return active_; }
    
    void spawn(Point pos) {
        position_ = pos;
        active_ = true;
    }
    
    void consume() {
        active_ = false;
    }

private:
    Point position_;
    bool active_;
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_ENTITIES_FOOD_HPP
