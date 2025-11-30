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
    int difficultyLevel;
    int currentDelay;
    int pointsSinceLastSpeedUp;
    int pointsSinceLastObstacle;
    char keyStroke;

public:

    Game(int level) {

        board = new Board();
        body = new Body();
        food = new Food();

        this->difficultyLevel = level;
        this->currentDelay = DELAY;
        this->pointsSinceLastSpeedUp = 0;
        this->pointsSinceLastObstacle = 0;

        // get first food point of the game!
        this->validateFood();
        
        // print Score, Size and Food initial position
        board->setPrintScore(difficultyLevel);
        board->setPrintSize(*body);
        board->setPrintFood(*food);
    
    }

    ~Game() { endwin(); }
    
    int getScore() const { return board->getScore(); }
    int getSnakeSize() const { return body->getSize(); }
    int getCurrentDelay() const { return currentDelay; }
    int getPointsSinceLastSpeedUp() const { return pointsSinceLastSpeedUp; }

    void validateFood() {
       food->getFood();
        if (board->getChar(*food) == '@' || board->getChar(*food) == '-' || board->getChar(*food) == '|') { // food born inside snake or wall
            validateFood();
        }
    }
    
    void spawnObstacle() {
        // Simple random obstacle generation
        int x = (rand() % (LINES - 2)) + 1;
        int y = (rand() % (COLS - 2)) + 1;
        Point p(x, y);
        
        // Don't spawn on snake, food, or existing walls
        char ch = board->getChar(p);
        if (ch != ' ' || (x == food->getX() && y == food->getY())) {
            return; // Try again next time or skip
        }
        
        board->printObstacle(p);
    }

    void reset() {

        delete board;
        delete body;
        delete food;

        clear();

        board = new Board();
        body = new Body();
        food = new Food();
        
        this->currentDelay = DELAY;
        this->pointsSinceLastSpeedUp = 0;
        this->pointsSinceLastObstacle = 0;

        // get first food point of the game!
        this->validateFood();

        // print Score, Size and Food initial position
        board->setPrintScore(difficultyLevel);
        board->setPrintSize(*body);
        board->setPrintFood(*food);
    }

    bool isGameOver() {

        if(clock.getTimestamp() >= currentDelay) {

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
              board->setPrintScore(difficultyLevel);
              board->setPrintSize(*body);
              
              // Progressive Difficulty Logic
              pointsSinceLastSpeedUp += difficultyLevel;
              pointsSinceLastObstacle += difficultyLevel;
              
              // Increase speed every 50 points
              if (pointsSinceLastSpeedUp >= 50) {
                  if (currentDelay > 30) { // Cap max speed
                      currentDelay -= 5;
                      // Visual feedback for speed up could be added here
                  }
                  pointsSinceLastSpeedUp = 0;
              }
              
              // Spawn obstacle every 100 points
              if (pointsSinceLastObstacle >= 100) {
                  spawnObstacle();
                  pointsSinceLastObstacle = 0;
              }
              
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