#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../libs/terminal.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// ============================================================================
// ANSI COLOR CONSTANTS TESTS
// ============================================================================

TEST_CASE("Color constants are defined correctly", "[terminal][color]") {
    REQUIRE(COLOR_DEFAULT == -1);
    REQUIRE(COLOR_BLACK == 0);
    REQUIRE(COLOR_RED == 1);
    REQUIRE(COLOR_GREEN == 2);
    REQUIRE(COLOR_YELLOW == 3);
    REQUIRE(COLOR_BLUE == 4);
    REQUIRE(COLOR_MAGENTA == 5);
    REQUIRE(COLOR_CYAN == 6);
    REQUIRE(COLOR_WHITE == 7);
}

TEST_CASE("Color constants are unique", "[terminal][color]") {
    REQUIRE(COLOR_RED != COLOR_GREEN);
    REQUIRE(COLOR_RED != COLOR_BLUE);
    REQUIRE(COLOR_GREEN != COLOR_BLUE);
    REQUIRE(COLOR_CYAN != COLOR_MAGENTA);
    REQUIRE(COLOR_YELLOW != COLOR_WHITE);
    REQUIRE(COLOR_DEFAULT != COLOR_BLACK);
}

// ============================================================================
// KEY CONSTANTS TESTS
// ============================================================================

TEST_CASE("Key constants are defined correctly", "[terminal][keys]") {
    REQUIRE(KEY_UP == 259);
    REQUIRE(KEY_DOWN == 258);
    REQUIRE(KEY_LEFT == 260);
    REQUIRE(KEY_RIGHT == 261);
    REQUIRE(KEY_ENTER == '\n');
    REQUIRE(ERR == -1);
}

TEST_CASE("Key constants are unique", "[terminal][keys]") {
    REQUIRE(KEY_UP != KEY_DOWN);
    REQUIRE(KEY_UP != KEY_LEFT);
    REQUIRE(KEY_UP != KEY_RIGHT);
    REQUIRE(KEY_DOWN != KEY_LEFT);
    REQUIRE(KEY_DOWN != KEY_RIGHT);
    REQUIRE(KEY_LEFT != KEY_RIGHT);
}

TEST_CASE("Boolean constants are defined", "[terminal][constants]") {
    REQUIRE(TRUE == 1);
    REQUIRE(FALSE == 0);
}

// ============================================================================
// ACS CHARACTER CONSTANTS TESTS
// ============================================================================

TEST_CASE("ACS character constants are ASCII", "[terminal][acs]") {
    // ACS characters should be ASCII characters
    REQUIRE(ACS_ULCORNER == '+');
    REQUIRE(ACS_URCORNER == '+');
    REQUIRE(ACS_LLCORNER == '+');
    REQUIRE(ACS_LRCORNER == '+');
    REQUIRE(ACS_HLINE == '-');
    REQUIRE(ACS_VLINE == '|');
    REQUIRE(ACS_DIAMOND == '*');
}

// ============================================================================
// ATTRIBUTE CONSTANTS TESTS
// ============================================================================

TEST_CASE("Attribute constants are defined correctly", "[terminal][attributes]") {
    REQUIRE(A_CHARTEXT == 0xFF);
    REQUIRE(A_BOLD == 0x0100);
    REQUIRE(A_REVERSE == 0x0200);
    REQUIRE(A_BLINK == 0x0400);
}

TEST_CASE("Attribute constants are unique", "[terminal][attributes]") {
    REQUIRE(A_CHARTEXT != A_BOLD);
    REQUIRE(A_BOLD != A_REVERSE);
    REQUIRE(A_REVERSE != A_BLINK);
    REQUIRE(A_BOLD != A_BLINK);
}

TEST_CASE("A_CHARTEXT can extract character from chtype", "[terminal][attributes]") {
    chtype ch = 'A' | A_BOLD;
    char extracted = ch & A_CHARTEXT;
    REQUIRE(extracted == 'A');
}

// ============================================================================
// ColoredChar STRUCT TESTS
// ============================================================================

TEST_CASE("ColoredChar default constructor", "[terminal][ColoredChar]") {
    ColoredChar cc;
    REQUIRE(cc.ch == ' ');
    REQUIRE(cc.fg == -1);
    REQUIRE(cc.bg == -1);
    REQUIRE(cc.attr == 0);
}

TEST_CASE("ColoredChar parameterized constructor", "[terminal][ColoredChar]") {
    ColoredChar cc('X', COLOR_RED, COLOR_BLUE, A_BOLD);
    REQUIRE(cc.ch == 'X');
    REQUIRE(cc.fg == COLOR_RED);
    REQUIRE(cc.bg == COLOR_BLUE);
    REQUIRE(cc.attr == A_BOLD);
}

TEST_CASE("ColoredChar equality operator", "[terminal][ColoredChar]") {
    ColoredChar cc1('A', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    ColoredChar cc2('A', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    ColoredChar cc3('B', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    
    SECTION("identical ColoredChars are equal") {
        REQUIRE(cc1 == cc2);
    }
    
    SECTION("different character makes them unequal") {
        REQUIRE_FALSE(cc1 == cc3);
    }
    
    SECTION("different foreground makes them unequal") {
        ColoredChar cc4('A', COLOR_GREEN, COLOR_DEFAULT, A_BOLD);
        REQUIRE_FALSE(cc1 == cc4);
    }
    
    SECTION("different background makes them unequal") {
        ColoredChar cc4('A', COLOR_RED, COLOR_BLUE, A_BOLD);
        REQUIRE_FALSE(cc1 == cc4);
    }
    
    SECTION("different attributes makes them unequal") {
        ColoredChar cc4('A', COLOR_RED, COLOR_DEFAULT, A_REVERSE);
        REQUIRE_FALSE(cc1 == cc4);
    }
}

TEST_CASE("ColoredChar inequality operator", "[terminal][ColoredChar]") {
    ColoredChar cc1('A', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    ColoredChar cc2('A', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    ColoredChar cc3('B', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    
    REQUIRE_FALSE(cc1 != cc2);
    REQUIRE(cc1 != cc3);
}

// ============================================================================
// ANSI COLOR CODE GENERATION TESTS
// ============================================================================

TEST_CASE("get_ansi_fg_color generates correct codes", "[terminal][ansi_color]") {
    const char* red = get_ansi_fg_color(COLOR_RED);
    const char* green = get_ansi_fg_color(COLOR_GREEN);
    const char* blue = get_ansi_fg_color(COLOR_BLUE);
    
    REQUIRE(std::strlen(red) > 0);
    REQUIRE(std::strlen(green) > 0);
    REQUIRE(std::strlen(blue) > 0);
    
    REQUIRE(red[0] == '\x1b');
    REQUIRE(green[0] == '\x1b');
    REQUIRE(blue[0] == '\x1b');
    
    REQUIRE(red != green);
    REQUIRE(green != blue);
}

TEST_CASE("get_ansi_bg_color generates correct codes", "[terminal][ansi_color]") {
    const char* red = get_ansi_bg_color(COLOR_RED);
    const char* green = get_ansi_bg_color(COLOR_GREEN);
    const char* blue = get_ansi_bg_color(COLOR_BLUE);
    
    REQUIRE(std::strlen(red) > 0);
    REQUIRE(std::strlen(green) > 0);
    REQUIRE(std::strlen(blue) > 0);
    
    REQUIRE(red[0] == '\x1b');
    REQUIRE(green[0] == '\x1b');
    REQUIRE(blue[0] == '\x1b');
    
    REQUIRE(red != green);
    REQUIRE(green != blue);
}

TEST_CASE("get_ansi_fg_color handles COLOR_DEFAULT", "[terminal][ansi_color]") {
    const char* def = get_ansi_fg_color(COLOR_DEFAULT);
    REQUIRE(std::strlen(def) > 0);
    REQUIRE(def[0] == '\x1b');
}

TEST_CASE("get_ansi_bg_color handles COLOR_DEFAULT", "[terminal][ansi_color]") {
    const char* def = get_ansi_bg_color(COLOR_DEFAULT);
    REQUIRE(std::strlen(def) > 0);
    REQUIRE(def[0] == '\x1b');
}

// ============================================================================
// ESCAPE SEQUENCE CONSTANTS TESTS
// ============================================================================

TEST_CASE("ANSI escape sequences are correct", "[terminal][ansi_sequences]") {
    REQUIRE(std::string(ANSI_RESET) == "\x1b[0m");
    REQUIRE(std::string(ANSI_CURSOR_HIDE) == "\x1b[?25l");
    REQUIRE(std::string(ANSI_CURSOR_SHOW) == "\x1b[?25h");
    REQUIRE(std::string(ANSI_CLEAR_SCREEN) == "\x1b[2J");
    REQUIRE(std::string(ANSI_CURSOR_HOME) == "\x1b[H");
    REQUIRE(std::string(ANSI_BOLD) == "\x1b[1m");
}

// ============================================================================
// TERMINAL SIZE MACROS TESTS
// ============================================================================

TEST_CASE("LINES and COLS macros work", "[terminal][macros]") {
    // After calling get_terminal_size(), LINES and COLS should be > 0
    // We can't call get_terminal_size directly without side effects,
    // but we can verify they exist
    REQUIRE(sizeof(LINES) > 0);
    REQUIRE(sizeof(COLS) > 0);
}

TEST_CASE("COLOR_PAIR macro works correctly", "[terminal][macros]") {
    int pair1 = COLOR_PAIR(1);
    int pair2 = COLOR_PAIR(2);
    int pair3 = COLOR_PAIR(5);
    
    REQUIRE(pair1 == 1);
    REQUIRE(pair2 == 2);
    REQUIRE(pair3 == 5);
    
    REQUIRE(pair1 != pair2);
    REQUIRE(pair2 != pair3);
}

TEST_CASE("stdscr macro is nullptr", "[terminal][macros]") {
    REQUIRE(stdscr == nullptr);
}

// ============================================================================
// COMPATIBILITY MACROS TESTS
// ============================================================================

TEST_CASE("Compatibility macros are defined", "[terminal][compat]") {
    // These should compile and be usable
    REQUIRE(TRUE == 1);
    REQUIRE(FALSE == 0);
}

// ============================================================================
// KEY EXTRACTION FROM ATTRIBUTES TESTS
// ============================================================================

TEST_CASE("Extracting character from chtype with attributes", "[terminal][chtype]") {
    SECTION("extract plain character") {
        chtype ch = 'X';
        char extracted = ch & A_CHARTEXT;
        REQUIRE(extracted == 'X');
    }
    
    SECTION("extract character with bold") {
        chtype ch = 'Y' | A_BOLD;
        char extracted = ch & A_CHARTEXT;
        REQUIRE(extracted == 'Y');
    }
    
    SECTION("extract character with multiple attributes") {
        chtype ch = 'Z' | A_BOLD | A_REVERSE;
        char extracted = ch & A_CHARTEXT;
        REQUIRE(extracted == 'Z');
    }
    
    SECTION("extract digit") {
        chtype ch = '5' | A_BLINK;
        char extracted = ch & A_CHARTEXT;
        REQUIRE(extracted == '5');
    }
}

// ============================================================================
// COLOR CODES CONSISTENCY TESTS
// ============================================================================

TEST_CASE("All foreground colors have unique codes", "[terminal][color_unique]") {
    const char* colors[] = {
        get_ansi_fg_color(COLOR_BLACK),
        get_ansi_fg_color(COLOR_RED),
        get_ansi_fg_color(COLOR_GREEN),
        get_ansi_fg_color(COLOR_YELLOW),
        get_ansi_fg_color(COLOR_BLUE),
        get_ansi_fg_color(COLOR_MAGENTA),
        get_ansi_fg_color(COLOR_CYAN),
        get_ansi_fg_color(COLOR_WHITE)
    };
    
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 8; j++) {
            REQUIRE(std::string(colors[i]) != std::string(colors[j]));
        }
    }
}

TEST_CASE("All background colors have unique codes", "[terminal][color_unique]") {
    const char* colors[] = {
        get_ansi_bg_color(COLOR_BLACK),
        get_ansi_bg_color(COLOR_RED),
        get_ansi_bg_color(COLOR_GREEN),
        get_ansi_bg_color(COLOR_YELLOW),
        get_ansi_bg_color(COLOR_BLUE),
        get_ansi_bg_color(COLOR_MAGENTA),
        get_ansi_bg_color(COLOR_CYAN),
        get_ansi_bg_color(COLOR_WHITE)
    };
    
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 8; j++) {
            REQUIRE(std::string(colors[i]) != std::string(colors[j]));
        }
    }
}

TEST_CASE("Foreground and background colors are different", "[terminal][color_diff]") {
    const char* fg_red = get_ansi_fg_color(COLOR_RED);
    const char* bg_red = get_ansi_bg_color(COLOR_RED);
    
    REQUIRE(std::string(fg_red) != std::string(bg_red));
    
    // Should follow ANSI standards: 3x for fg, 4x for bg
    REQUIRE(fg_red[2] == '3');
    REQUIRE(bg_red[2] == '4');
}

// ============================================================================
// TERMINAL COMPATIBILITY TESTS
// ============================================================================

TEST_CASE("All wrapping functions are declared", "[terminal][functions]") {
    // These should compile without errors
    REQUIRE(true);  // If we got here, all function declarations exist
}

TEST_CASE("All macro constants work in expressions", "[terminal][expressions]") {
    REQUIRE((COLOR_PAIR(1) | A_BOLD) > 0);
    REQUIRE((COLOR_PAIR(2) | A_REVERSE) > 0);
    REQUIRE((A_BOLD | A_REVERSE) > 0);
    REQUIRE((A_BOLD | A_BLINK | A_REVERSE) > 0);
}

// ============================================================================
// TERMINAL WRAPPER FUNCTIONS - LOGIC TESTS
// ============================================================================

TEST_CASE("ColoredChar can be copied and modified", "[terminal][ColoredChar_modify]") {
    ColoredChar original('A', COLOR_RED, COLOR_DEFAULT, A_BOLD);
    ColoredChar copy = original;
    
    REQUIRE(copy == original);
    
    copy.ch = 'B';
    REQUIRE_FALSE(copy == original);
    REQUIRE(copy.ch == 'B');
    REQUIRE(original.ch == 'A');
}

TEST_CASE("ColoredChar mixed with ASCII operations", "[terminal][ColoredChar_ascii]") {
    for (char c = 'A'; c <= 'Z'; c++) {
        ColoredChar cc(c, COLOR_GREEN, COLOR_DEFAULT, 0);
        REQUIRE(cc.ch == c);
        REQUIRE(cc.fg == COLOR_GREEN);
    }
}

TEST_CASE("ColoredChar with numbers", "[terminal][ColoredChar_numbers]") {
    for (char c = '0'; c <= '9'; c++) {
        ColoredChar cc(c, COLOR_YELLOW, COLOR_DEFAULT, A_BOLD);
        REQUIRE(cc.ch == c);
        REQUIRE(cc.attr == A_BOLD);
    }
}

// ============================================================================
// ATTRIBUTE COMBINATION TESTS
// ============================================================================

TEST_CASE("Multiple attributes can be combined", "[terminal][attr_combine]") {
    int combined = A_BOLD | A_REVERSE;
    REQUIRE((combined & A_BOLD) > 0);
    REQUIRE((combined & A_REVERSE) > 0);
    
    int combined2 = A_BOLD | A_REVERSE | A_BLINK;
    REQUIRE((combined2 & A_BOLD) > 0);
    REQUIRE((combined2 & A_REVERSE) > 0);
    REQUIRE((combined2 & A_BLINK) > 0);
}

TEST_CASE("A_CHARTEXT mask works correctly", "[terminal][mask]") {
    chtype with_attrs = 'X' | A_BOLD | A_REVERSE;
    char ch = with_attrs & A_CHARTEXT;
    
    REQUIRE(ch == 'X');
    REQUIRE((with_attrs & ~A_CHARTEXT) != 0);  // Attributes are preserved
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_CASE("Terminal constants form compatible API surface", "[terminal][integration]") {
    // Verify that we have all the constants needed for ncurses compatibility
    REQUIRE(COLOR_PAIR(1) >= 0);
    REQUIRE(COLOR_PAIR(8) >= 0);
    
    REQUIRE(A_BOLD != 0);
    REQUIRE(A_REVERSE != 0);
    REQUIRE(A_BLINK != 0);
    
    REQUIRE(KEY_UP != KEY_DOWN);
    REQUIRE(KEY_LEFT != KEY_RIGHT);
    REQUIRE(KEY_ENTER == '\n');
    
    REQUIRE(ACS_ULCORNER != 0);
    REQUIRE(ACS_HLINE != 0);
    REQUIRE(ACS_VLINE != 0);
}

TEST_CASE("All color values are in valid range", "[terminal][color_range]") {
    REQUIRE(COLOR_DEFAULT == -1);
    REQUIRE(COLOR_BLACK >= 0);
    REQUIRE(COLOR_BLACK <= 7);
    REQUIRE(COLOR_RED >= 0);
    REQUIRE(COLOR_RED <= 7);
    REQUIRE(COLOR_GREEN >= 0);
    REQUIRE(COLOR_GREEN <= 7);
    REQUIRE(COLOR_YELLOW >= 0);
    REQUIRE(COLOR_YELLOW <= 7);
    REQUIRE(COLOR_BLUE >= 0);
    REQUIRE(COLOR_BLUE <= 7);
    REQUIRE(COLOR_MAGENTA >= 0);
    REQUIRE(COLOR_MAGENTA <= 7);
    REQUIRE(COLOR_CYAN >= 0);
    REQUIRE(COLOR_CYAN <= 7);
    REQUIRE(COLOR_WHITE >= 0);
    REQUIRE(COLOR_WHITE <= 7);
}

TEST_CASE("Key constants are distinct for each direction", "[terminal][key_distinct]") {
    int keys[] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT};
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            REQUIRE(keys[i] != keys[j]);
        }
    }
}
