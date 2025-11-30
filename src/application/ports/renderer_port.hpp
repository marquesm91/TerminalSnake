/**
 * @file renderer_port.hpp
 * @brief IRenderer - Interface for rendering (Output Port)
 * 
 * Clean Architecture: Application Layer - Output Port
 * Hexagonal: Port (interface exposed by the application)
 */

#ifndef APPLICATION_PORTS_RENDERER_PORT_HPP
#define APPLICATION_PORTS_RENDERER_PORT_HPP

#include <vector>
#include "domain/value_objects/point.hpp"
#include "domain/entities/snake.hpp"
#include "domain/entities/food.hpp"
#include "application/ports/leaderboard_port.hpp"

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Interface for rendering
 * 
 * Defines contract for any visual output adapter:
 * - NCurses (terminal)
 * - OLED display (Arduino)
 * - SDL/OpenGL (desktop GUI)
 * - etc.
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    // Lifecycle
    virtual void init() = 0;
    virtual void shutdown() = 0;
    
    // Frame control
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    // Drawing primitives
    virtual void clear() = 0;
    virtual void drawBorder(uint8_t width, uint8_t height) = 0;
    virtual void drawSnake(const Domain::SnakeEntity& snake) = 0;
    virtual void drawFood(const Domain::FoodEntity& food) = 0;
    virtual void drawScore(uint32_t score, uint32_t highscore) = 0;
    virtual void drawPaused() = 0;
    virtual void drawGameOver(uint32_t score, uint32_t highscore) = 0;
    
    // Menu/UI
    virtual void drawMenu(int selectedOption, uint32_t highscore) = 0;
    virtual void drawLeaderboard(const std::vector<LeaderboardEntry>& entries = {}) = 0;
    virtual void drawMessage(const char* message) = 0;
    
    // Screen info
    virtual int screenWidth() const = 0;
    virtual int screenHeight() const = 0;
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_RENDERER_PORT_HPP
