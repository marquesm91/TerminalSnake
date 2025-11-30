#include <stdio.h>
#include <assert.h>
#include "../web/web_curses.hpp"

// Mock Board::getChar logic
char getCharMock(int y, int x) {
    chtype ch = mvinch(y, x);
    char c = ch & A_CHARTEXT;
    
    // Check for borders (ACS characters)
    if ((ch & A_CHARTEXT) == (ACS_HLINE & A_CHARTEXT)) return '-';
    
    return c;
}

int main() {
    printf("Running WASM Collision Test (With Color)...\n");
    
    initscr();
    
    // Scenario: Draw a horizontal line with COLOR_BORDER (1) and BOLD
    // This matches Board::drawGameBorder()
    int color_border = 1;
    attron(COLOR_PAIR(color_border) | A_BOLD);
    mvaddch(0, 0, ACS_HLINE);
    attroff(COLOR_PAIR(color_border) | A_BOLD);
    
    // Verify what mvinch returns
    chtype ch = mvinch(0, 0);
    printf("Debug Info:\n");
    printf("ACS_HLINE: 0x%X\n", ACS_HLINE);
    printf("COLOR_PAIR(1): 0x%X\n", COLOR_PAIR(1));
    printf("A_BOLD: 0x%X\n", A_BOLD);
    printf("A_CHARTEXT: 0x%X\n", A_CHARTEXT);
    printf("mvinch(0,0): 0x%X\n", ch);
    printf("ch & A_CHARTEXT: 0x%X\n", ch & A_CHARTEXT);
    printf("ACS_HLINE & A_CHARTEXT: 0x%X\n", ACS_HLINE & A_CHARTEXT);
    
    char result = getCharMock(0, 0);
    printf("getChar result: '%c' (0x%X)\n", result, result);
    
    if (result == '-') {
        printf("PASS: Wall detected correctly.\n");
        return 0;
    } else {
        printf("FAIL: Wall NOT detected! Expected '-', got 0x%X\n", result);
        return 1;
    }
}
