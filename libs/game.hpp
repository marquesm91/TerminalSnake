#ifndef GAME_H_
#define GAME_H_

#include "common.hpp"
#include "clock.hpp"

#include "point.hpp"
#include "food.hpp"
#include "body.hpp"
#include "board.hpp"


class Game{

    Board *board;
    Body *body;
    Food *food;
    Clock clock;
    int level;
    char keyStroke;

public:

    Game(int level) {

        board = new Board();
        body = new Body();
        food = new Food();

        this->level = level;

        // get first food point of the game!
        this->validateFood();
        
        // print Score, Size and Food initial position
        board->setPrintScore(level);
        board->setPrintSize(*body);
        board->setPrintFood(*food);
    
    }

    ~Game() { endwin(); }

    void validateFood() {
       food->getFood();
        if (board->getChar(*food) == '@') { // food born inside snake
            validateFood();
        }
    }

    void reset() {

        delete board;
        delete body;
        delete food;

        clear();

        board = new Board();
        body = new Body();
        food = new Food();

        // get first food point of the game!
        this->validateFood();

        // print Score, Size and Food initial position
        board->setPrintScore(level);
        board->setPrintSize(*body);
        board->setPrintFood(*food);
    }

    bool isGameOver() {

        if(clock.getTimestamp() >= DELAY) {

            keyStroke = getch();
            body->validateDirection(keyStroke);

            Point newHead = body->investigatePosition();
            char ch = board->getChar(newHead);

            if (ch == '@' || ch == '-' || ch == '|') { // Snake cant move!
                
                board->printGameOver();
                board->update();
                return true;
            
            } else if (ch == 'f') { // Snake can eat and move!
            
              this->validateFood();

              body->setHead(newHead);
              board->setPrintSnake(*body);

              board->setPrintFood(*food);
              board->setPrintScore(level);
              board->setPrintSize(*body);
              
              board->update();
            
            } else { //Snake can move!

              body->setHead(newHead);
              board->setPrintSnake(*body);
              body->removeTail();
              board->update();
            }

            clock.reset();
        }

        return false;
    }
};

#endif