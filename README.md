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
$ touch ~/.bash_aliases && echo 'alias tsnake="TSNAKE_DIR"' >> ~/.bash_aliases<br>
$ source ~/.bashrc
```

## Design Game and Functionalities

To be written...

## How the game works

<<<<<<< HEAD
To be written...
=======
<img src="https://github.com/marquesm91/TerminalSnake/blob/master/layout_v1.0.0.png" width="500">
>>>>>>> 72c71b6a808c777b10c8b270459c12fa678bffda

## Versions

* v1.0.0 - Implemented the movement, limit board and generate the food randomly. 
* v1.1.0 - First version of the game.
* v1.1.1 - Introduced the unit-tests using Catch.
* v1.1.2 - Improved the game design architecture.
* <strong>v1.1.3</strong> - Correct a bug where the food <em>f</em> born inside the snake <em>@</em>.

### Need to improve

<<<<<<< HEAD
* Remove the usleep() used because it can delay the movement snake when holding the key pressed.
* Create the Highscore functionalite.
* Integrate a database to store the Highscore.
=======
Tip: if you want to make <code>tsnake</code> a default command on your terminal you can do:<br>
<code>touch ~/.bash_aliases ... (learn full command)</code>
>>>>>>> 72c71b6a808c777b10c8b270459c12fa678bffda