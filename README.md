# Terminal Snake

The old and good Snake game now available to play in Terminal.

![v1.0](http://i.imgur.com/pokjYD5.png)

## Before you start

This game were coded using <a href="https://github.com/gittup/ncurses">NCurses</a> an library to ease the development of Console Applications. You can make a game window or print any character on anywhere with just a few code lines. Run `$ sudo apt-get install libncurses5-dev` to install it correctly.

All the tests were coded using <a href="https://github.com/philsquared/Catch">Catch</a> a powerful framework for unit-tests and it is header-only! So you will see a `catch.hpp` being used inside `tests` folder.

## Build and play

Run the following commands:

```
$ git clone https://github.com/marquesm91/TerminalSnake
$ cd TerminalSnake
$ make
$ ./bin/tsnake
```

If you want to set `tsnake` a default command on your terminal you may run these next commands replacing TSNAKE_DIR with your `tsnake` file path, i.e `~/mygames/TerminalSnake/bin/tsnake`.

```
$ touch ~/.bash_aliases && echo 'alias tsnake="TSNAKE_DIR"' >> ~/.bash_aliases
$ source ~/.bashrc
```

## Game Design and Functionalities

To be written...

## How the game works

To be written...

## Versions

* v1.0 - First version of the game. 
* v1.0a - Introduced the unit-tests using Catch.
* v1.0b - Improved the game design architecture.
* v1.0c - Correct a bug where the food <em>f</em> born inside the snake <em>@</em>.
* v1.0d - Introduced the Clock and now a timestamp could be created to control the snake moves and forget the usleep().
* v1.0e - Add a difference between delay UP/DOWN movements and RIGHT/LEFT. This make the game more fluid.
* v1.1 - Whole code refactored. Introduced Clock, Board, Game and Common. Body describes only the Snake which before described the game.
* v1.1a - Fix a bug where size and score aren't printing correctly.
* v1.2 - Introduced the GAME OVER screen and ask if you want to play again.
* <strong>v1.2a</strong> - Fix a bug when you press two arrow keys rapidly and the second is prohibited in relation to the actual direction of the snake the game ends and show the game over screen.

### Need to do

* Create the Highscore functionalite.
* Integrate a database to store the Highscore.
* Menu for the game (change difficulty, change board, etc)