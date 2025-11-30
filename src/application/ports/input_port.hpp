/**
 * @file input_port.hpp
 * @brief IInput - Interface for user input (Input Port)
 * 
 * Clean Architecture: Application Layer - Input Port
 */

#ifndef APPLICATION_PORTS_INPUT_PORT_HPP
#define APPLICATION_PORTS_INPUT_PORT_HPP

#include "domain/value_objects/direction.hpp"

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Tipos de comando de entrada
 */
enum class InputCommand {
    None,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Pause,
    Quit,
    Confirm,
    Back,
    MenuUp,
    MenuDown
};

/**
 * @brief Interface for user input
 * 
 * Abstracts the input source:
 * - Keyboard (ncurses)
 * - Physical buttons (Arduino)
 * - Gamepad
 * - etc.
 */
class IInput {
public:
    virtual ~IInput() = default;
    
    /**
     * @brief Check if input is available
     */
    virtual bool hasInput() const = 0;
    
    /**
     * @brief Read next command (non-blocking)
     */
    virtual InputCommand readCommand() = 0;
    
    /**
     * @brief Read next command (blocking)
     */
    virtual InputCommand waitForCommand() = 0;
    
    /**
     * @brief Convert command to direction
     */
    static Domain::Direction commandToDirection(InputCommand cmd) {
        switch (cmd) {
            case InputCommand::MoveUp:    return Domain::Direction::Up;
            case InputCommand::MoveDown:  return Domain::Direction::Down;
            case InputCommand::MoveLeft:  return Domain::Direction::Left;
            case InputCommand::MoveRight: return Domain::Direction::Right;
            default: return Domain::Direction::None;
        }
    }
    
    /**
     * @brief Check if command is a movement
     */
    static bool isMovementCommand(InputCommand cmd) {
        return cmd == InputCommand::MoveUp ||
               cmd == InputCommand::MoveDown ||
               cmd == InputCommand::MoveLeft ||
               cmd == InputCommand::MoveRight;
    }
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_INPUT_PORT_HPP
