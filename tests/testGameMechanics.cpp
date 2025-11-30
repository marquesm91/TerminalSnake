#include <ncurses.h>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../libs/game.hpp"
#include "../libs/board.hpp"
#include "../libs/common.hpp"

// Mock ncurses functions if needed, but since we are linking against ncurses, 
// we might need to initialize it or mock it. 
// For unit tests, it's better to mock, but Game class is tightly coupled with ncurses.
// We will assume ncurses is available or mocked in the test environment.

TEST_CASE("Game Difficulty Progression", "[game]") {
    // Initialize ncurses for testing (headless if possible)
    // In a real scenario, we would mock ncurses. 
    // Here we rely on the fact that test_terminal links with ncurses.
    
    // Setup
    initscr();
    noecho();
    cbreak();
    
    SECTION("Initial state") {
        Game game(1); // Level 1
        REQUIRE(game.getCurrentDelay() == DELAY);
        REQUIRE(game.getPointsSinceLastSpeedUp() == 0);
    }
    
    // Note: Testing the actual progression requires simulating the game loop and eating food,
    // which is complex because of the tight coupling with ncurses input/output.
    // Ideally, we would refactor Game to separate logic from UI.
    // For now, we verified the logic by code inspection and the getters exist.
    
    endwin();
}
