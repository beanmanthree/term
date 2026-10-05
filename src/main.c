#include <stdio.h>

#include "ansi.h"
#include "term.h"

int main(void) {
    ANSI_clearScreen();
    ANSI_moveTo(1, 1);
    ANSI_fg256(196);
    printf("Hello, ANSI Colors!\n");
    ANSI_fg256(46);
    printf("This is a test of 256 colors.\n");
    ANSI_fgRgb(0, 0, 255);
    printf("And this is a test of RGB colors.\n");
    ANSI_showCursor();
    return 0;
}