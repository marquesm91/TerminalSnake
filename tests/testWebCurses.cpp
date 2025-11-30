#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Define __EMSCRIPTEN__ to enable web_curses.hpp content
#define __EMSCRIPTEN__

// Include mocks
#include "mocks/emscripten.h"
#include "mocks/emscripten/html5.h"

// Include the file to test
#include "../web/web_curses.hpp"

#include <string>
#include <vector>

// Implement mocked JS functions
std::string last_written;
std::vector<std::string> write_history;
int last_key = -1;
int term_cols = 0;
int term_rows = 0;
bool cursor_visible = true;

// Implement the mocked functions declared by EM_JS macro
void js_terminal_write(const char* str) {
    last_written = str;
    write_history.push_back(str);
}

void js_terminal_clear() {
    last_written = "CLEAR";
    write_history.push_back("CLEAR");
}

void js_terminal_set_size(int cols, int rows) {
    term_cols = cols;
    term_rows = rows;
}

int js_get_key() {
    return last_key;
}

void js_hide_cursor() {
    cursor_visible = false;
}

void js_show_cursor() {
    cursor_visible = true;
}

// Helper to reset state
void reset_mock_state() {
    last_written = "";
    write_history.clear();
    last_key = -1;
    term_cols = 0;
    term_rows = 0;
    cursor_visible = true;
    
    // Suppress unused variable warnings
    (void)term_cols;
    (void)term_rows;
    
    if (stdscr) {
        endwin();
    }
}

TEST_CASE("WebCurses Initialization", "[web_curses]") {
    reset_mock_state();
    
    initscr();
    REQUIRE(stdscr != nullptr);
    REQUIRE(LINES == 24);
    REQUIRE(COLS == 80);
    REQUIRE(term_cols == 80);
    REQUIRE(term_rows == 24);
    REQUIRE(cursor_visible == false);
    
    endwin();
    REQUIRE(stdscr == nullptr);
    REQUIRE(cursor_visible == true);
}

TEST_CASE("WebCurses Add Character", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    // addch should update buffer but not write to terminal until refresh
    addch('A');
    REQUIRE(WebCurses::screenBuffer[0][0].ch == 'A');
    REQUIRE(WebCurses::bufferDirty == true);
    
    // refresh should flush buffer
    refresh();
    REQUIRE(WebCurses::bufferDirty == false);
    REQUIRE(write_history.size() > 0);
    
    endwin();
}

TEST_CASE("WebCurses Move Cursor", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    move(10, 20);
    REQUIRE(stdscr->curY == 10);
    REQUIRE(stdscr->curX == 20);
    
    addch('X');
    REQUIRE(WebCurses::screenBuffer[10][20].ch == 'X');
    
    endwin();
}

TEST_CASE("WebCurses Colors", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLUE);
    
    REQUIRE(WebCurses::colorPairs[1][0] == COLOR_RED);
    REQUIRE(WebCurses::colorPairs[1][1] == COLOR_BLUE);
    
    attron(COLOR_PAIR(1));
    addch('C');
    REQUIRE(WebCurses::screenBuffer[0][0].colorPair == 1);
    
    attroff(COLOR_PAIR(1));
    addch('D');
    REQUIRE(WebCurses::screenBuffer[0][1].colorPair == 0); // Assuming 0 is default
    
    endwin();
}

TEST_CASE("WebCurses Input", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    // Test queue input
    web_curses_push_key('A');
    int ch = getch();
    REQUIRE(ch == 'A');
    
    // Test JS input
    last_key = 'B';
    ch = getch();
    REQUIRE(ch == 'B');
    
    // Test no input
    last_key = -1;
    nodelay(stdscr, TRUE);
    ch = getch();
    REQUIRE(ch == ERR);
    
    endwin();
}

TEST_CASE("WebCurses Clear", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    addch('X');
    clear();
    
    REQUIRE(WebCurses::screenBuffer[0][0].ch == ' ');
    REQUIRE(last_written == "CLEAR");
    
    endwin();
}

TEST_CASE("WebCurses Read Screen (mvinch)", "[web_curses]") {
    reset_mock_state();
    initscr();
    
    // Write something to screen
    mvaddch(5, 5, 'Z');
    
    // Verify cursor position
    REQUIRE(stdscr->curY == 5);
    REQUIRE(stdscr->curX == 6); // Advanced by 1
    
    // Verify buffer has it directly
    // Note: screenBuffer is static in WebCurses namespace, accessible here
    REQUIRE(WebCurses::screenBuffer[5][5].ch == 'Z');
    
    // Verify mvinch can read it back
    chtype ch = mvinch(5, 5);
    REQUIRE((ch & A_CHARTEXT) == 'Z');
    
    // Verify reading empty space
    ch = mvinch(0, 0);
    REQUIRE((ch & A_CHARTEXT) == ' ');
    
    endwin();
}
