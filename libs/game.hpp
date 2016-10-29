#ifndef GAME_H_
#define GAME_H_

#include "common.hpp"
#include "clock.hpp"

#include "point.hpp"
#include "food.hpp"
#include "body.hpp"
#include "board.hpp"


class Game{

    Board board;
    Body body;
    Food food;
    Clock clock;
    int level;
    char keyStroke;

public:

    Game(int level) {

        this->level = level;

        // get first food point of the game!
        this->validateFood();
        
        // print Score, Size and Food initial position
        board.setPrintScore(level);
        board.setPrintSize(body);
        board.setPrintFood(food);
    
    }

    ~Game() { endwin(); }

    void validateFood() {
       food.getFood();
        if (board.getChar(food) == '@') { // food born inside snake
            validateFood();
        }
    }

    bool isGameOver() {

        keyStroke = getch();
        body.validateDirection(keyStroke);

        if(clock.getTimestamp() >= DELAY) {
            
            Point newHead = body.investigatePosition();
            char ch = board.getChar(newHead);

            if (ch == '@' || ch == '-' || ch == '|') { // Snake cant move!
                
                return true;
            
            } else if (ch == 'f') { // Snake can eat and move!
            
              this->validateFood();

              body.setHead(newHead);
              board.setPrintSnake(body);

              board.setPrintFood(food);
              board.setPrintScore(level);
              board.setPrintSize(body);
              
              board.update();
            
            } else { //Snake can move!

              body.setHead(newHead);
              board.setPrintSnake(body);
              body.removeTail();
              board.update();
            }

            clock.reset();
        }

        return false;
    }
};

#endif