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

// Key codes - MUST match common.hpp: UP=3, DOWN=2, LEFT=4, RIGHT=5
#define KEY_UP      3
#define KEY_DOWN    2
#define KEY_LEFT    4
#define KEY_RIGHT   5
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

// Attributes - Shifted to avoid conflict with Unicode chars (0-0xFFFF)
#define A_NORMAL    0
#define A_BOLD      (1 << 16)
#define A_REVERSE   (1 << 17)
#define A_UNDERLINE (1 << 18)
#define A_BLINK     (1 << 19)

// Color pair in bits 24-31
#define COLOR_PAIR(n) (((n) & 0xFF) << 24)
#define PAIR_NUMBER(a) (((a) >> 24) & 0xFF)

// A_CHARTEXT mask for extracting character from chtype (16 bits for BMP)
#define A_CHARTEXT 0xFFFF

// ACS characters - Use Unicode box-drawing characters for better visuals
// These work well with xterm.js and modern browsers
#define ACS_ULCORNER 0x250C  // ┌
#define ACS_URCORNER 0x2510  // ┐
#define ACS_LLCORNER 0x2514  // └
#define ACS_LRCORNER 0x2518  // ┘
#define ACS_HLINE    0x2500  // ─
#define ACS_VLINE    0x2502  // │
#define ACS_LTEE     0x251C  // ├
#define ACS_RTEE     0x2524  // ┤
#define ACS_TTEE     0x252C  // ┬
#define ACS_BTEE     0x2534  // ┴
#define ACS_PLUS     0x253C  // ┼
#define ACS_BLOCK    0x2588  // █ Full block
#define ACS_DIAMOND  0x25C6  // ◆
#define ACS_CKBOARD  0x2592  // ▒
#define ACS_BULLET   0x25CF  // ●

// ============================================================================
// Types
// ============================================================================

typedef unsigned int chtype;
typedef chtype attr_t;

// Screen cell: character + attributes
struct ScreenCell {
    chtype ch;       // Character (may be Unicode codepoint)
    attr_t attrs;    // Attributes (color, bold, etc)
    int colorPair;   // Color pair number
    
    ScreenCell() : ch(' '), attrs(0), colorPair(0) {}
};

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
    
    // Screen buffer - maintains state between refreshes
    static ScreenCell screenBuffer[24][80];
    static bool bufferDirty = true;
    
    // Color pairs: [pair_num] = {fg, bg}
    static int colorPairs[64][2];
    static int currentColorPair = 0;
    static attr_t currentAttrs = 0;
}

// Forward declarations
inline int addch(chtype ch);
inline int move(int y, int x);

// ============================================================================
// Input Queue API (called from main_web.cpp)
// ============================================================================

inline void web_curses_push_key(int key) {
    WebCurses::inputQueue.push(key);
}

// ============================================================================
// JavaScript Bridge
// ============================================================================

EM_JS(void, js_terminal_write, (const char* str), {
    if (typeof window !== 'undefined' && window.terminalWrite) {
        window.terminalWrite(UTF8ToString(str));
    }
});

EM_JS(void, js_terminal_clear, (), {
    if (typeof window !== 'undefined' && window.terminalClear) {
        window.terminalClear();
    }
});

EM_JS(void, js_terminal_set_size, (int cols, int rows), {
    if (typeof window !== 'undefined' && window.terminalSetSize) {
        window.terminalSetSize(cols, rows);
    }
});

EM_JS(int, js_get_key, (), {
    if (typeof window !== 'undefined' && window.getKey) {
        return window.getKey();
    }
    return -1;
});

EM_JS(void, js_hide_cursor, (), {
    if (typeof window !== 'undefined' && window.terminalWrite) {
        window.terminalWrite("\x1b[?25l");
    }
});

EM_JS(void, js_show_cursor, (), {
    if (typeof window !== 'undefined' && window.terminalWrite) {
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
    
    // Initialize screen buffer with spaces
    for (int y = 0; y < WebCurses::LINES; y++) {
        for (int x = 0; x < WebCurses::COLS; x++) {
            WebCurses::screenBuffer[y][x].ch = ' ';
            WebCurses::screenBuffer[y][x].attrs = 0;
            WebCurses::screenBuffer[y][x].colorPair = 0;
        }
    }
    WebCurses::bufferDirty = true;
    
    js_terminal_set_size(WebCurses::COLS, WebCurses::LINES);
    js_hide_cursor();
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

// Helper to convert Unicode codepoint to UTF-8
inline void codepoint_to_utf8(unsigned int cp, char* buf) {
    if (cp < 0x80) {
        buf[0] = static_cast<char>(cp);
        buf[1] = '\0';
    } else if (cp < 0x800) {
        buf[0] = static_cast<char>(0xC0 | (cp >> 6));
        buf[1] = static_cast<char>(0x80 | (cp & 0x3F));
        buf[2] = '\0';
    } else if (cp < 0x10000) {
        buf[0] = static_cast<char>(0xE0 | (cp >> 12));
        buf[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = static_cast<char>(0x80 | (cp & 0x3F));
        buf[3] = '\0';
    } else {
        buf[0] = static_cast<char>(0xF0 | (cp >> 18));
        buf[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (cp & 0x3F));
        buf[4] = '\0';
    }
}

inline int clear() {
    js_terminal_clear();
    // Clear the buffer too
    for (int y = 0; y < WebCurses::LINES; y++) {
        for (int x = 0; x < WebCurses::COLS; x++) {
            WebCurses::screenBuffer[y][x].ch = ' ';
            WebCurses::screenBuffer[y][x].attrs = 0;
            WebCurses::screenBuffer[y][x].colorPair = 0;
        }
    }
    WebCurses::bufferDirty = true;
    return OK;
}

// Helper function to render buffer to terminal
inline void renderBuffer() {
    // Move to home position
    js_terminal_write("\x1b[H");
    
    char utf8buf[5];
    int lastColorPair = -1;
    attr_t lastAttrs = 0;
    
    for (int y = 0; y < WebCurses::LINES; y++) {
        for (int x = 0; x < WebCurses::COLS; x++) {
            ScreenCell& cell = WebCurses::screenBuffer[y][x];
            
            // Apply color/attribute changes if needed
            if (cell.colorPair != lastColorPair || cell.attrs != lastAttrs) {
                // Reset attributes
                js_terminal_write("\x1b[0m");
                
                // Apply color pair
                if (cell.colorPair > 0 && cell.colorPair < 64) {
                    int fg = WebCurses::colorPairs[cell.colorPair][0];
                    int bg = WebCurses::colorPairs[cell.colorPair][1];
                    
                    char colorBuf[32];
                    if (fg >= 0 && fg <= 7) {
                        snprintf(colorBuf, sizeof(colorBuf), "\x1b[%dm", 30 + fg);
                        js_terminal_write(colorBuf);
                    }
                    if (bg >= 0 && bg <= 7) {
                        snprintf(colorBuf, sizeof(colorBuf), "\x1b[%dm", 40 + bg);
                        js_terminal_write(colorBuf);
                    }
                }
                
                // Apply bold
                if (cell.attrs & A_BOLD) {
                    js_terminal_write("\x1b[1m");
                }
                
                lastColorPair = cell.colorPair;
                lastAttrs = cell.attrs;
            }
            
            // Write character
            codepoint_to_utf8(cell.ch & 0xFFFFFF, utf8buf);
            js_terminal_write(utf8buf);
        }
        // Move to next line
        if (y < WebCurses::LINES - 1) {
            js_terminal_write("\r\n");
        }
    }
    
    // Reset attributes
    js_terminal_write("\x1b[0m");
}

inline int refresh() {
    if (WebCurses::bufferDirty) {
        renderBuffer();
        WebCurses::bufferDirty = false;
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
    
    // Extract color pair
    int pair = PAIR_NUMBER(attrs);
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
    
    // Update currentColorPair based on remaining attributes
    WebCurses::currentColorPair = PAIR_NUMBER(WebCurses::currentAttrs);

    // Reset all attributes and reapply remaining
    js_terminal_write("\x1b[0m");
    if (WebCurses::currentAttrs) {
        attron(WebCurses::currentAttrs);
    }
    return OK;
}

inline int mvprintw(int y, int x, const char* fmt, ...) {
    move(y, x);
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    // Write each character to the buffer
    for (int i = 0; buf[i] != '\0'; i++) {
        addch(static_cast<unsigned char>(buf[i]));
    }
    return OK;
}

inline int printw(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    // Write each character to the buffer
    for (int i = 0; buf[i] != '\0'; i++) {
        addch(static_cast<unsigned char>(buf[i]));
    }
    return OK;
}

inline int move(int y, int x) {
    // Just update cursor position in our buffer
    if (WebCurses::stdscr) {
        WebCurses::stdscr->curY = y;
        WebCurses::stdscr->curX = x;
    }
    return OK;
}

inline int addch(chtype ch) {
    // Write to screen buffer instead of directly to terminal
    int y = WebCurses::stdscr ? WebCurses::stdscr->curY : 0;
    int x = WebCurses::stdscr ? WebCurses::stdscr->curX : 0;
    
    if (y >= 0 && y < WebCurses::LINES && x >= 0 && x < WebCurses::COLS) {
        WebCurses::screenBuffer[y][x].ch = ch;
        WebCurses::screenBuffer[y][x].attrs = WebCurses::currentAttrs;
        WebCurses::screenBuffer[y][x].colorPair = WebCurses::currentColorPair;
        WebCurses::bufferDirty = true;
        
        // Advance cursor
        if (WebCurses::stdscr) {
            WebCurses::stdscr->curX++;
            if (WebCurses::stdscr->curX >= WebCurses::COLS) {
                WebCurses::stdscr->curX = 0;
                WebCurses::stdscr->curY++;
            }
        }
    }
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
    // First check internal queue (keys pushed from handleInput)
    if (!WebCurses::inputQueue.empty()) {
        int key = WebCurses::inputQueue.front();
        WebCurses::inputQueue.pop();
        return key;
    }
    
    // Then check JavaScript queue
    int key = js_get_key();
    
    // Handle timeout/nodelay
    if (key == -1 && WebCurses::nodelay_mode) {
        return ERR;
    }
    
    return key;
}

// Additional ncurses functions needed by the game

inline chtype mvinch(int y, int x) {
    if (y >= 0 && y < WebCurses::LINES && x >= 0 && x < WebCurses::COLS) {
        // Return character ORed with attributes and color pair
        // Reconstruct attributes from stored state
        attr_t attrs = WebCurses::screenBuffer[y][x].attrs;
        int pair = WebCurses::screenBuffer[y][x].colorPair;
        
        return WebCurses::screenBuffer[y][x].ch | attrs | COLOR_PAIR(pair);
    }
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

// Global variables accessible from game code
#define stdscr WebCurses::stdscr
#define LINES WebCurses::LINES
#define COLS WebCurses::COLS

#endif // __EMSCRIPTEN__

#endif // WEB_CURSES_HPP
