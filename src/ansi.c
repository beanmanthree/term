#include "ansi.h"

#include <limits.h>
#include <stdio.h>

#include "term.h"
#include "util.h"

static void ANSI_moveRel(const char *code, int n) {
    if (n < 0) n = 0;
    printf(ANSI_CSI "%d%s", n, code);
}

void ANSI_flush(void) {
    fflush(stdout);
}

void ANSI_fg256(int idx) {
    idx = UTIL_clampInt(idx, 0, 255);
    printf(ANSI_CSI "38;5;%dm", idx);
}

void ANSI_bg256(int idx) {
    idx = UTIL_clampInt(idx, 0, 255);
    printf(ANSI_CSI "48;5;%dm", idx);
}

void ANSI_fgRgb(int r, int g, int b) {
    printf(ANSI_CSI "38;2;%d;%d;%dm",
           UTIL_clampInt(r, 0, 255),
           UTIL_clampInt(g, 0, 255),
           UTIL_clampInt(b, 0, 255));
}

void ANSI_bgRgb(int r, int g, int b) {
    printf(ANSI_CSI "48;2;%d;%d;%dm",
           UTIL_clampInt(r, 0, 255),
           UTIL_clampInt(g, 0, 255),
           UTIL_clampInt(b, 0, 255));
}

void ANSI_moveUp(int n) {
    ANSI_moveRel("A", n);
}

void ANSI_moveDown(int n) {
    ANSI_moveRel("B", n);
}

void ANSI_moveForward(int n) {
    ANSI_moveRel("C", n);
}

void ANSI_moveBackward(int n) {
    ANSI_moveRel("D", n);
}

void ANSI_moveTo(int r, int c) {
    printf(ANSI_CSI "%d;%dH", UTIL_clampInt(r, 1, INT_MAX), UTIL_clampInt(c, 1, INT_MAX));
}

void ANSI_clearScreen(void) {
    printf(ANSI_CSI "2J");
}

void ANSI_hideCursor(void) {
    printf(ANSI_CSI "?25l");
}

void ANSI_showCursor(void) {
    printf(ANSI_CSI "?25h");
}

void ANSI_saveCursor(void) {
    printf(ANSI_CSI "s");
}

void ANSI_restoreCursor(void) {
    printf(ANSI_CSI "u");
}

void ANSI_enableMouse(void) {
    printf(ANSI_CSI "?1003h" ANSI_CSI "?1006h");
    fflush(stdout);
}

void ANSI_disableMouse(void) {
    printf(ANSI_CSI "?1003l" ANSI_CSI "?1006l");
    fflush(stdout);
}

static int read_decimal(unsigned int *value, int delimiter, int *terminator) {
    int ch = TERM_getch();
    if (ch < '0' || ch > '9') return -1;

    unsigned int result = 0;
    do {
        unsigned int digit = (unsigned int)(ch - '0');
        if (result > (UINT_MAX - digit) / 10) return -1;
        result = result * 10 + digit;
        ch = TERM_getch();
    } while (ch >= '0' && ch <= '9');

    if (delimiter == -1 ? (ch != 'M' && ch != 'm') : ch != delimiter) return -1;
    *value = result;
    if (terminator != NULL) *terminator = ch;
    return 0;
}

int ANSI_getMouseEvent(ANSI_MouseEvent *event) {
    if (event == NULL) return -1;

    int ch;
    for (;;) {
        ch = TERM_getch();
        if (ch == EOF) return -1;
        if (ch != '\x1b') continue;

        ch = TERM_getch();
        if (ch == EOF) return -1;
        if (ch != '[') continue;

        ch = TERM_getch();
        if (ch == EOF) return -1;
        if (ch != '<') continue;
        break;
    }

    unsigned int code, x, y;
    if (read_decimal(&code, ';', NULL) == -1 ||
        read_decimal(&x, ';', NULL) == -1 ||
        read_decimal(&y, -1, &ch) == -1 ||
        x == 0 || y == 0) {
        return -1;
    }

    event->button = code & 3U;
    event->x = x;
    event->y = y;
    event->modifiers = code & (ANSI_MOUSE_SHIFT | ANSI_MOUSE_ALT | ANSI_MOUSE_CTRL);
    event->motion = (code & 32U) != 0;
    event->wheel = (code & 64U) != 0;
    event->release = ch == 'm';
    return 1;
}