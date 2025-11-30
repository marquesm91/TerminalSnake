/**
 * @file web_curses.hpp
 * @brief Minimal ncurses-compatible implementation for WebAssembly/Emscripten
 * 
 * This provides a subset of ncurses functions that work with xterm.js in the browser.
 * It maps ncurses calls to terminal escape sequences.
 */

#ifndef WEB_CURSES_HPP
#define WEB_CURSES_HPP

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <queue>

// ============================================================================
// Constants
// ============================================================================

#define ERR (-1)
#define OK 0
#define TRUE 1
#define FALSE 0

// Key codes
#define KEY_UP      0x103
#define KEY_DOWN    0x102
#define KEY_LEFT    0x104
#define KEY_RIGHT   0x105
#define KEY_ENTER   0x157

// Colors
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7
#define COLOR_DEFAULT -1

// Attributes
#define A_NORMAL    0
#define A_BOLD      (1 << 8)
#define A_REVERSE   (1 << 9)
#define A_UNDERLINE (1 << 10)

// ACS characters (box drawing)
#define ACS_ULCORNER '+'
#define ACS_URCORNER '+'
#define ACS_LLCORNER '+'
#define ACS_LRCORNER '+'
#define ACS_HLINE    '-'
#define ACS_VLINE    '|'

// ============================================================================
// Types
// ============================================================================

typedef unsigned int chtype;
typedef chtype attr_t;

struct WINDOW {
    int rows;
    int cols;
    int curY;
    int curX;
    attr_t attrs;
    int colorPair;
};

// ============================================================================
// Global State
// ============================================================================

namespace WebCurses {
    static WINDOW* stdscr = nullptr;
    static int LINES = 24;
    static int COLS = 80;
    static bool nodelay_mode = false;
    static int timeout_ms = -1;
    static std::queue<int> inputQueue;
    
    // Color pairs: [pair_num] = {fg, bg}
    static int colorPairs[64][2];
    static int currentColorPair = 0;
    static attr_t currentAttrs = 0;
}

// ============================================================================
// JavaScript Bridge
// ============================================================================

EM_JS(void, js_terminal_write, (const char* str), {
    if (window.terminalWrite) {
        window.terminalWrite(UTF8ToString(str));
    }
});

EM_JS(void, js_terminal_clear, (), {
    if (window.terminalClear) {
        window.terminalClear();
    }
});

EM_JS(void, js_terminal_set_size, (int cols, int rows), {
    if (window.terminalSetSize) {
        window.terminalSetSize(cols, rows);
    }
});

EM_JS(int, js_get_key, (), {
    if (window.getKey) {
        return window.getKey();
    }
    return -1;
});

EM_JS(void, js_hide_cursor, (), {
    if (window.terminalWrite) {
        window.terminalWrite("\x1b[?25l");
    }
});

EM_JS(void, js_show_cursor, (), {
    if (window.terminalWrite) {
        window.terminalWrite("\x1b[?25h");
    }
});

// ============================================================================
// ncurses-compatible functions
// ============================================================================

inline WINDOW* initscr() {
    WebCurses::stdscr = new WINDOW();
    WebCurses::stdscr->rows = WebCurses::LINES;
    WebCurses::stdscr->cols = WebCurses::COLS;
    WebCurses::stdscr->curY = 0;
    WebCurses::stdscr->curX = 0;
    WebCurses::stdscr->attrs = A_NORMAL;
    WebCurses::stdscr->colorPair = 0;
    
    js_terminal_set_size(WebCurses::COLS, WebCurses::LINES);
    return WebCurses::stdscr;
}

inline int endwin() {
    js_show_cursor();
    if (WebCurses::stdscr) {
        delete WebCurses::stdscr;
        WebCurses::stdscr = nullptr;
    }
    return OK;
}

inline int start_color() {
    return OK;
}

inline int use_default_colors() {
    return OK;
}

inline int init_pair(int pair, int fg, int bg) {
    if (pair >= 0 && pair < 64) {
        WebCurses::colorPairs[pair][0] = fg;
        WebCurses::colorPairs[pair][1] = bg;
    }
    return OK;
}

inline int cbreak() {
    return OK;
}

inline int noecho() {
    return OK;
}

inline int curs_set(int visibility) {
    if (visibility == 0) {
        js_hide_cursor();
    } else {
        js_show_cursor();
    }
    return OK;
}

inline int keypad(WINDOW* /* win */, bool /* bf */) {
    return OK;
}

inline int nodelay(WINDOW* /* win */, bool bf) {
    WebCurses::nodelay_mode = bf;
    return OK;
}

inline void timeout(int delay) {
    WebCurses::timeout_ms = delay;
}

inline int clear() {
    js_terminal_clear();
    return OK;
}

inline int refresh() {
    // Terminal is immediately updated, no buffering needed
    return OK;
}

inline int move(int y, int x) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y + 1, x + 1);
    js_terminal_write(buf);
    if (WebCurses::stdscr) {
        WebCurses::stdscr->curY = y;
        WebCurses::stdscr->curX = x;
    }
    return OK;
}

// Get ANSI color code
inline const char* getAnsiColor(int color, bool foreground) {
    static char buf[16];
    int base = foreground ? 30 : 40;
    
    if (color == COLOR_DEFAULT || color < 0) {
        snprintf(buf, sizeof(buf), "%d", foreground ? 39 : 49);
    } else {
        snprintf(buf, sizeof(buf), "%d", base + color);
    }
    return buf;
}

inline int attron(attr_t attrs) {
    WebCurses::currentAttrs |= attrs;
    
    // Extract color pair from attrs (lower 8 bits might be color pair)
    int pair = attrs & 0xFF;
    if (pair > 0 && pair < 64) {
        WebCurses::currentColorPair = pair;
    }
    
    // Build ANSI escape sequence
    char buf[64];
    char* p = buf;
    p += sprintf(p, "\x1b[");
    
    bool first = true;
    
    if (attrs & A_BOLD) {
        p += sprintf(p, "1");
        first = false;
    }
    if (attrs & A_REVERSE) {
        if (!first) *p++ = ';';
        p += sprintf(p, "7");
        first = false;
    }
    if (attrs & A_UNDERLINE) {
        if (!first) *p++ = ';';
        p += sprintf(p, "4");
        first = false;
    }
    
    // Apply color pair
    if (pair > 0 && pair < 64) {
        int fg = WebCurses::colorPairs[pair][0];
        int bg = WebCurses::colorPairs[pair][1];
        if (!first) *p++ = ';';
        p += sprintf(p, "%s", getAnsiColor(fg, true));
        *p++ = ';';
        p += sprintf(p, "%s", getAnsiColor(bg, false));
    }
    
    *p++ = 'm';
    *p = '\0';
    
    js_terminal_write(buf);
    return OK;
}

inline int attroff(attr_t attrs) {
    WebCurses::currentAttrs &= ~attrs;
    // Reset all attributes and reapply remaining
    js_terminal_write("\x1b[0m");
    if (WebCurses::currentAttrs) {
        attron(WebCurses::currentAttrs);
    }
    return OK;
}

#define COLOR_PAIR(n) ((n) & 0xFF)

inline int printw(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    js_terminal_write(buf);
    return OK;
}

inline int mvprintw(int y, int x, const char* fmt, ...) {
    move(y, x);
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    js_terminal_write(buf);
    return OK;
}

inline int addch(chtype ch) {
    char buf[2] = { static_cast<char>(ch & 0xFF), '\0' };
    js_terminal_write(buf);
    return OK;
}

inline int mvaddch(int y, int x, chtype ch) {
    move(y, x);
    return addch(ch);
}

inline int mvhline(int y, int x, chtype ch, int n) {
    move(y, x);
    for (int i = 0; i < n; i++) {
        addch(ch);
    }
    return OK;
}

inline int mvvline(int y, int x, chtype ch, int n) {
    for (int i = 0; i < n; i++) {
        mvaddch(y + i, x, ch);
    }
    return OK;
}

inline int getch() {
    int key = js_get_key();
    
    // Handle timeout/nodelay
    if (key == -1 && WebCurses::nodelay_mode) {
        return ERR;
    }
    
    return key;
}

// Additional ncurses functions needed by the game

inline chtype mvinch(int y, int x) {
    // This function reads a character from the screen
    // In our web implementation, we can't easily read back from xterm.js
    // The game should track screen state separately
    return ' ';
}

inline int wrefresh(WINDOW* /* win */) {
    return refresh();
}

inline int wclear(WINDOW* /* win */) {
    return clear();
}

inline int box(WINDOW* /* win */, chtype /* verch */, chtype /* horch */) {
    // Draw a box around the window - for stdscr
    mvaddch(0, 0, ACS_ULCORNER);
    mvaddch(0, WebCurses::COLS - 1, ACS_URCORNER);
    mvaddch(WebCurses::LINES - 1, 0, ACS_LLCORNER);
    mvaddch(WebCurses::LINES - 1, WebCurses::COLS - 1, ACS_LRCORNER);
    
    for (int j = 1; j < WebCurses::COLS - 1; j++) {
        mvaddch(0, j, ACS_HLINE);
        mvaddch(WebCurses::LINES - 1, j, ACS_HLINE);
    }
    
    for (int i = 1; i < WebCurses::LINES - 1; i++) {
        mvaddch(i, 0, ACS_VLINE);
        mvaddch(i, WebCurses::COLS - 1, ACS_VLINE);
    }
    
    return OK;
}

inline int getmaxy(WINDOW* /* win */) {
    return WebCurses::LINES;
}

inline int getmaxx(WINDOW* /* win */) {
    return WebCurses::COLS;
}

// A_BLINK attribute (not really supported in most terminals)
#define A_BLINK (1 << 11)

// A_CHARTEXT mask for extracting character from chtype
#define A_CHARTEXT 0xFF

// ACS_DIAMOND for food
#define ACS_DIAMOND '*'

// Global variables accessible from game code
#define stdscr WebCurses::stdscr
#define LINES WebCurses::LINES
#define COLS WebCurses::COLS

#endif // __EMSCRIPTEN__

#endif // WEB_CURSES_HPP
