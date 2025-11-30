/**
 * @file ncurses_input.hpp
 * @brief NCursesInput - Adaptador de entrada para terminal
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 */

#ifndef INFRASTRUCTURE_ADAPTERS_INPUT_NCURSES_INPUT_HPP
#define INFRASTRUCTURE_ADAPTERS_INPUT_NCURSES_INPUT_HPP

#include <ncurses.h>
#include "application/ports/input_port.hpp"

namespace Snake {
namespace Infrastructure {
namespace Adapters {

/**
 * @brief Adaptador de entrada NCurses
 */
class NCursesInput : public Application::Ports::IInput {
public:
    NCursesInput() = default;
    
    bool hasInput() const override {
        int ch = getch();
        if (ch != ERR) {
            ungetch(ch);  // Devolve o caractere
            return true;
        }
        return false;
    }
    
    Application::Ports::InputCommand readCommand() override {
        int ch = getch();
        return keyToCommand(ch);
    }
    
    Application::Ports::InputCommand waitForCommand() override {
        nodelay(stdscr, FALSE);  // Blocking mode
        int ch = getch();
        nodelay(stdscr, TRUE);   // Non-blocking mode
        return keyToCommand(ch);
    }

private:
    Application::Ports::InputCommand keyToCommand(int key) {
        switch (key) {
            case KEY_UP:
            case 'w':
            case 'W':
            case 3:  // Compatibilidade com common.hpp
                return Application::Ports::InputCommand::MoveUp;
                
            case KEY_DOWN:
            case 's':
            case 'S':
            case 2:
                return Application::Ports::InputCommand::MoveDown;
                
            case KEY_LEFT:
            case 'a':
            case 'A':
            case 4:
                return Application::Ports::InputCommand::MoveLeft;
                
            case KEY_RIGHT:
            case 'd':
            case 'D':
            case 5:
                return Application::Ports::InputCommand::MoveRight;
                
            case 'p':
            case 'P':
            case ' ':
                return Application::Ports::InputCommand::Pause;
                
            case 'q':
            case 'Q':
            case 27:  // ESC
                return Application::Ports::InputCommand::Quit;
                
            case '\n':
            case '\r':
                return Application::Ports::InputCommand::Confirm;
                
            case ERR:
            default:
                return Application::Ports::InputCommand::None;
        }
    }
};

} // namespace Adapters
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_ADAPTERS_INPUT_NCURSES_INPUT_HPP
