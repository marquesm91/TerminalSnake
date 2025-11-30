#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <ctime>
#include <cctype>
#include <cstdarg>

// ============================================================================
// TYPE DEFINITIONS AND ACS DEFINITIONS
// ============================================================================

typedef unsigned int chtype;

#define A_CHARTEXT 0xFF
#define A_BOLD 0x0100
#define A_REVERSE 0x0200
#define A_BLINK 0x0400

// ACS characters (mapped to ASCII equivalents for ANSI terminals)
#define ACS_ULCORNER '+'
#define ACS_URCORNER '+'
#define ACS_LLCORNER '+'
#define ACS_LRCORNER '+'
#define ACS_HLINE '-'
#define ACS_VLINE '|'
#define ACS_DIAMOND '*'

// ============================================================================
// COLOR CONSTANTS (Compatible with NCurses)
// ============================================================================

#define COLOR_DEFAULT -1
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

// ============================================================================
// KEY CONSTANTS (Compatible with NCurses)
// ============================================================================

#define KEY_UP 259
#define KEY_DOWN 258
#define KEY_LEFT 260
#define KEY_RIGHT 261
#define KEY_ENTER '\n'
#define ERR -1

// Boolean constants for compatibility
#define TRUE 1
#define FALSE 0

// ============================================================================
// ANSI/VT100 ESCAPE SEQUENCES
// ============================================================================

// Cursor control
#define ANSI_CURSOR_HOME     "\x1b[H"
#define ANSI_CURSOR_HIDE     "\x1b[?25l"
#define ANSI_CURSOR_SHOW     "\x1b[?25h"
#define ANSI_CLEAR_SCREEN    "\x1b[2J"
#define ANSI_RESET           "\x1b[0m"
#define ANSI_BOLD            "\x1b[1m"

// ============================================================================
// COLOR STRUCTURE - Store char with color info
// ============================================================================

struct ColoredChar {
    char ch;
    int fg;  // foreground color
    int bg;  // background color
    int attr; // attributes (BOLD, etc)
    
    ColoredChar() : ch(' '), fg(-1), bg(-1), attr(0) {}
    ColoredChar(char c, int f, int b, int a) : ch(c), fg(f), bg(b), attr(a) {}
    
    bool operator==(const ColoredChar& other) const {
        return ch == other.ch && fg == other.fg && bg == other.bg && attr == other.attr;
    }
    
    bool operator!=(const ColoredChar& other) const {
        return !(*this == other);
    }
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

static struct termios g_original_termios;
static int g_lines = 24;
static int g_cols = 80;
static ColoredChar** g_screen_buffer = nullptr;
static ColoredChar** g_previous_buffer = nullptr;
static bool g_initialized = false;

// Current attributes being applied
static int g_current_fg = -1;
static int g_current_bg = -1;
static int g_current_attr = 0;

// ============================================================================
// ANSI COLOR HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Get ANSI color code for foreground color
 */
const char* get_ansi_fg_color(int color) {
    switch (color) {
        case COLOR_RED: return "\x1b[31m";
        case COLOR_GREEN: return "\x1b[32m";
        case COLOR_YELLOW: return "\x1b[33m";
        case COLOR_BLUE: return "\x1b[34m";
        case COLOR_MAGENTA: return "\x1b[35m";
        case COLOR_CYAN: return "\x1b[36m";
        case COLOR_WHITE: return "\x1b[37m";
        case COLOR_BLACK: return "\x1b[30m";
        default: return "\x1b[39m";  // default
    }
}

/**
 * @brief Get ANSI color code for background color
 */
const char* get_ansi_bg_color(int color) {
    switch (color) {
        case COLOR_RED: return "\x1b[41m";
        case COLOR_GREEN: return "\x1b[42m";
        case COLOR_YELLOW: return "\x1b[43m";
        case COLOR_BLUE: return "\x1b[44m";
        case COLOR_MAGENTA: return "\x1b[45m";
        case COLOR_CYAN: return "\x1b[46m";
        case COLOR_WHITE: return "\x1b[47m";
        case COLOR_BLACK: return "\x1b[40m";
        default: return "\x1b[49m";  // default
    }
}

/**
 * @brief Apply color and attributes to terminal
 */
void apply_ansi_color(int fg, int bg, int attr) {
    printf(ANSI_RESET);  // Always reset first
    
    if (fg >= 0) {
        printf("%s", get_ansi_fg_color(fg));
    }
    
    if (bg >= 0) {
        printf("%s", get_ansi_bg_color(bg));
    }
    
    if (attr & A_BOLD) {
        printf(ANSI_BOLD);
    }
    
    if (attr & A_REVERSE) {
        printf("\x1b[7m");
    }
    
    if (attr & A_BLINK) {
        printf("\x1b[5m");
    }
}

// ============================================================================
// TERMINAL CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Restores the original terminal settings
 */
void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_original_termios);
}

/**
 * @brief Configures the terminal to raw/non-blocking mode
 */
void set_raw_mode() {
    tcgetattr(STDIN_FILENO, &g_original_termios);
    atexit(restore_terminal);

    struct termios raw = g_original_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);

    // Set non-blocking read
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/**
 * @brief Gets the current terminal dimensions
 */
void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    g_lines = w.ws_row;
    g_cols = w.ws_col;
}

/**
 * @brief Moves cursor to position (y, x)
 */
void go_to_xy(int y, int x) {
    printf("\x1b[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

/**
 * @brief Clears the entire screen
 */
void clear_screen() {
    printf(ANSI_CLEAR_SCREEN);
    printf(ANSI_CURSOR_HOME);
    fflush(stdout);
}

/**
 * @brief Hides the cursor
 */
void hide_cursor() {
    printf(ANSI_CURSOR_HIDE);
    fflush(stdout);
}

/**
 * @brief Shows the cursor
 */
void show_cursor() {
    printf(ANSI_CURSOR_SHOW);
    fflush(stdout);
}

/**
 * @brief Resets all attributes
 */
void reset_attributes() {
    printf(ANSI_RESET);
    fflush(stdout);
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * @brief Handles special keys (arrow keys)
 */
int handle_special_keys() {
    char c;
    
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return -1;
    }
    
    // Check for ESC sequence start
    if (c == '\x1b') {
        if (read(STDIN_FILENO, &c, 1) != 1) {
            return -1;
        }
        
        if (c == '[') {
            if (read(STDIN_FILENO, &c, 1) != 1) {
                return -1;
            }
            
            // Map arrow keys to NCurses equivalents
            switch (c) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                default: return -1;
            }
        }
    }
    
    return (unsigned char)c;
}

/**
 * @brief Gets input with arrow key support
 */
int getch() {
    return handle_special_keys();
}

/**
 * @brief Timeout (compatibility function)
 */
void timeout(int ms) {
    (void)ms;
}

// ============================================================================
// SCREEN BUFFER MANAGEMENT
// ============================================================================

/**
 * @brief Initializes the screen buffer system
 */
void init_screen() {
    get_terminal_size();
    
    // Allocate current buffer
    g_screen_buffer = new ColoredChar*[g_lines];
    for (int i = 0; i < g_lines; i++) {
        g_screen_buffer[i] = new ColoredChar[g_cols];
        for (int j = 0; j < g_cols; j++) {
            g_screen_buffer[i][j] = ColoredChar(' ', -1, -1, 0);
        }
    }
    
    // Allocate previous buffer for diff
    g_previous_buffer = new ColoredChar*[g_lines];
    for (int i = 0; i < g_lines; i++) {
        g_previous_buffer[i] = new ColoredChar[g_cols];
        for (int j = 0; j < g_cols; j++) {
            g_previous_buffer[i][j] = ColoredChar('\0', -1, -1, 0);
        }
    }
    
    g_initialized = true;
}

/**
 * @brief Frees the screen buffers
 */
void cleanup_screen() {
    if (!g_initialized) return;
    
    for (int i = 0; i < g_lines; i++) {
        delete[] g_screen_buffer[i];
        delete[] g_previous_buffer[i];
    }
    delete[] g_screen_buffer;
    delete[] g_previous_buffer;
    g_screen_buffer = nullptr;
    g_previous_buffer = nullptr;
    g_initialized = false;
}

/**
 * @brief Renders only the differences with colors
 */
void refresh_diff() {
    for (int y = 0; y < g_lines; y++) {
        for (int x = 0; x < g_cols; x++) {
            if (g_screen_buffer[y][x] != g_previous_buffer[y][x]) {
                go_to_xy(y, x);
                
                // Apply color
                apply_ansi_color(g_screen_buffer[y][x].fg, 
                                g_screen_buffer[y][x].bg, 
                                g_screen_buffer[y][x].attr);
                
                // Print character
                printf("%c", g_screen_buffer[y][x].ch);
                
                // Reset after each character
                printf(ANSI_RESET);
            }
        }
    }
    
    // Copy current to previous for next refresh
    for (int y = 0; y < g_lines; y++) {
        for (int x = 0; x < g_cols; x++) {
            g_previous_buffer[y][x] = g_screen_buffer[y][x];
        }
    }
    
    fflush(stdout);
}

// ============================================================================
// NCurses-compatible WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Initializes the terminal (equivalent to initscr)
 */
void initscr() {
    set_raw_mode();
    get_terminal_size();
    init_screen();
    clear_screen();
    reset_attributes();
    fflush(stdout);
}

/**
 * @brief Initializes a color pair (compatibility function)
 */
void init_pair(int pair, int fg, int bg) {
    // No-op: colors are handled dynamically in attron()
    (void)pair;
    (void)fg;
    (void)bg;
}

/**
 * @brief Enables color support (equivalent to start_color)
 */
void start_color() {
    // Already supported in ANSI mode
}

/**
 * @brief Enables cbreak mode
 */
void cbreak() {
    // Already handled by set_raw_mode
}

/**
 * @brief Disables echo
 */
void noecho() {
    // Already handled by set_raw_mode
}

/**
 * @brief Sets nodelay mode (non-blocking input)
 */
void nodelay(int* stdscr, bool flag) {
    (void)stdscr;
    (void)flag;
}

/**
 * @brief Sets cursor visibility
 */
void curs_set(int visibility) {
    if (visibility == 0) {
        hide_cursor();
    } else {
        show_cursor();
    }
}

/**
 * @brief Enables keypad mode (arrow keys)
 */
void keypad(int* stdscr, bool flag) {
    (void)stdscr;
    (void)flag;
}

/**
 * @brief Ends terminal mode
 */
void endwin() {
    show_cursor();
    reset_attributes();
    cleanup_screen();
    restore_terminal();
    clear_screen();
}

/**
 * @brief Clears the screen
 */
void clear() {
    for (int i = 0; i < g_lines; i++) {
        for (int j = 0; j < g_cols; j++) {
            g_screen_buffer[i][j] = ColoredChar(' ', -1, -1, 0);
        }
    }
    
    // Clear previous buffer to force full redraw
    for (int i = 0; i < g_lines; i++) {
        for (int j = 0; j < g_cols; j++) {
            g_previous_buffer[i][j] = ColoredChar('\0', -1, -1, 0);
        }
    }
}

/**
 * @brief Full clear of terminal and buffers (use before game starts)
 */
void full_clear_screen() {
    printf(ANSI_CLEAR_SCREEN);
    printf(ANSI_CURSOR_HOME);
    reset_attributes();
    fflush(stdout);
    
    // Reset all buffers
    for (int i = 0; i < g_lines; i++) {
        for (int j = 0; j < g_cols; j++) {
            g_screen_buffer[i][j] = ColoredChar(' ', -1, -1, 0);
            g_previous_buffer[i][j] = ColoredChar('\0', -1, -1, 0);
        }
    }
}

/**
 * @brief Refreshes the display
 */
void refresh() {
    refresh_diff();
}

/**
 * @brief Turns on attributes (COLOR_PAIR, A_BOLD, etc)
 */
void attron(int attr) {
    int color_pair = attr & 0xFF;
    
    // Color pairs: 1=CYAN, 2=GREEN, 3=GREEN, 4=RED, 5=YELLOW, 6=MAGENTA, 7=RED, 8=WHITE+BLUE
    switch (color_pair) {
        case 1:
            g_current_fg = COLOR_CYAN;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 2:
        case 3:
            g_current_fg = COLOR_GREEN;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 4:
            g_current_fg = COLOR_RED;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 5:
            g_current_fg = COLOR_YELLOW;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 6:
            g_current_fg = COLOR_MAGENTA;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 7:
            g_current_fg = COLOR_RED;
            g_current_bg = COLOR_DEFAULT;
            break;
        case 8:
            g_current_fg = COLOR_WHITE;
            g_current_bg = COLOR_BLUE;
            break;
        default:
            g_current_fg = -1;
            g_current_bg = -1;
    }
    
    g_current_attr = attr & ~0xFF;  // Store only attributes, not color pair
    
    // Send to terminal for direct output functions (printw, etc)
    apply_ansi_color(g_current_fg, g_current_bg, g_current_attr);
}

/**
 * @brief Turns off attributes
 */
void attroff(int attr) {
    (void)attr;
    g_current_fg = -1;
    g_current_bg = -1;
    g_current_attr = 0;
    reset_attributes();
}

/**
 * @brief Moves to position and prints formatted string
 */
int mvprintw(int y, int x, const char* fmt, ...) {
    if (y < 0 || y >= g_lines || x < 0 || x >= g_cols) {
        return 0;
    }
    
    va_list args;
    va_start(args, fmt);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Copy to screen buffer with current colors
    int len = std::strlen(buffer);
    for (int i = 0; i < len && (x + i) < g_cols; i++) {
        g_screen_buffer[y][x + i].ch = buffer[i];
        g_screen_buffer[y][x + i].fg = g_current_fg;
        g_screen_buffer[y][x + i].bg = g_current_bg;
        g_screen_buffer[y][x + i].attr = g_current_attr;
    }
    
    return len;
}

/**
 * @brief Prints formatted string at current position with color
 */
int printw(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Apply current color and print
    apply_ansi_color(g_current_fg, g_current_bg, g_current_attr);
    printf("%s", buffer);
    fflush(stdout);
    
    return std::strlen(buffer);
}

/**
 * @brief Moves to position and adds character
 */
void mvaddch(int y, int x, chtype ch) {
    if (y < 0 || y >= g_lines || x < 0 || x >= g_cols) {
        return;
    }
    
    g_screen_buffer[y][x].ch = ch & A_CHARTEXT;
    g_screen_buffer[y][x].fg = g_current_fg;
    g_screen_buffer[y][x].bg = g_current_bg;
    g_screen_buffer[y][x].attr = g_current_attr;
}

/**
 * @brief Gets character at position
 */
chtype mvinch(int y, int x) {
    if (y < 0 || y >= g_lines || x < 0 || x >= g_cols) {
        return ' ';
    }
    
    return (chtype)g_screen_buffer[y][x].ch;
}

/**
 * @brief Uses default colors (no-op in ANSI mode)
 */
void use_default_colors() {
    // Already using default colors in ANSI mode
}

// ============================================================================
// MACROS
// ============================================================================

#define LINES (g_lines)
#define COLS (g_cols)
#define COLOR_PAIR(n) (n)
#define stdscr nullptr

#endif // TERMINAL_H_
