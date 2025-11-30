# Terminal Snake 🐍

The old and good Snake game now available to play in Terminal with a modern interface!

![v1.0](http://i.imgur.com/pokjYD5.png)

## ✨ Features

- **Modern Interface**: Colorful UI with status bar, styled borders, and game over screen
- **Menu System**: Navigate through main menu, settings, and difficulty options
- **Highscore System**: Persistent highscore saved locally
- **Multiple Difficulty Levels**: Easy, Normal, Hard, and Insane modes
- **Smooth Gameplay**: Optimized timing for fluid snake movement

## 📋 Prerequisites

This game was coded using [NCurses](https://github.com/gittup/ncurses), a library to ease the development of Console Applications. You can create game windows or print any character anywhere with just a few lines of code.

### Installing NCurses

**Ubuntu/Debian:**
```bash
sudo apt-get install libncurses5-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install ncurses-devel
```

**macOS:**
```bash
brew install ncurses
```

All the tests were coded using [Catch](https://github.com/philsquared/Catch), a powerful framework for unit-tests that is header-only! You will find `catch.hpp` in the `tests` folder.

## 🚀 Build and Play

Run the following commands:

```bash
git clone https://github.com/marquesm91/TerminalSnake
cd TerminalSnake
make
./bin/tsnake
```

### Adding as Terminal Command

If you want to set `tsnake` as a default command on your terminal, run these commands (replace `TSNAKE_DIR` with your actual path):

```bash
echo 'alias tsnake="~/TerminalSnake/bin/tsnake"' >> ~/.bash_aliases
source ~/.bashrc
```

## 🎮 Game Controls

| Key | Action |
|-----|--------|
| ↑ | Move Up |
| ↓ | Move Down |
| ← | Move Left |
| → | Move Right |
| Y/Enter | Confirm (Play again) |
| N | Decline (Exit) |
| Q | Quit/Back |

## 🏗️ Game Architecture

The game is organized into several modular components:

```
TerminalSnake/
├── main.cpp          # Entry point and game loop
├── Makefile          # Build configuration
├── libs/
│   ├── common.hpp    # Common constants and definitions
│   ├── point.hpp     # Point class for 2D coordinates
│   ├── clock.hpp     # Timestamp management for game timing
│   ├── food.hpp      # Food generation and positioning
│   ├── body.hpp      # Snake body management (movement, growth)
│   ├── board.hpp     # Game board rendering and collision detection
│   ├── game.hpp      # Main game logic controller
│   ├── menu.hpp      # Menu system interface
│   └── highscore.hpp # Persistent highscore management
└── tests/
    ├── catch.hpp     # Catch testing framework
    ├── Makefile      # Test build configuration
    └── testPoint.cpp # Unit tests for Point class
```

### Core Classes

| Class | Description |
|-------|-------------|
| `Point` | Base class representing 2D coordinates (x, y) |
| `Food` | Extends Point, handles random food generation |
| `Body` | Manages snake segments using a linked list |
| `Board` | Handles all rendering: borders, snake, food, UI |
| `Clock` | Provides timestamp-based game timing |
| `Game` | Main game controller, orchestrates all components |
| `Menu` | Interactive menu system with navigation |
| `Highscore` | Loads/saves highscore to file system |

### Game Flow

1. **Initialization**: NCurses setup, color pairs, menu display
2. **Menu Loop**: User selects Start Game, Settings, or Exit
3. **Game Loop**:
   - Read keyboard input
   - Validate direction change
   - Calculate new head position
   - Check collisions (wall, self)
   - Update snake position
   - Check food consumption
   - Render frame
4. **Game Over**: Display score, check highscore, prompt replay

## 🧪 Running Tests

```bash
cd tests
make
./t1
```

## 📦 Releases

| Version | Description |
|---------|-------------|
| **v1.4** | Snake now grows when eating food, size display updates correctly |
| v1.3 | Added menu system, settings, highscore persistence, modern UI |
| v1.2a | Fix bug when pressing two arrow keys rapidly |
| v1.2 | Introduced GAME OVER screen and play again prompt |
| v1.1a | Fix bug where size and score aren't printing correctly |
| v1.1 | Code refactored. Introduced Clock, Board, Game and Common |
| v1.0e | Add difference between UP/DOWN and RIGHT/LEFT delay |
| v1.0d | Introduced Clock for timestamp-based movement control |
| v1.0c | Fix bug where food spawns inside snake |
| v1.0b | Improved game design architecture |
| v1.0a | Introduced unit tests using Catch |
| v1.0 | First version of the game |

## 🗺️ Roadmap

- [x] Create highscore functionality
- [x] Menu system for game settings
- [x] Difficulty levels
- [ ] Multiple game modes (walls, obstacles)
- [ ] Sound effects
- [ ] Multiplayer support
- [ ] Custom themes/skins

## 📄 License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to submit issues and pull requests.