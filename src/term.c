#include "term.h"

#include <stdio.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>

    static HANDLE terminal_input;
    static DWORD saved_console_mode;
    static int raw_mode_enabled;

    int TERM_enableRawMode(void) {
        if (raw_mode_enabled) return 0;

        terminal_input = GetStdHandle(STD_INPUT_HANDLE);
        if (terminal_input == INVALID_HANDLE_VALUE || terminal_input == NULL) return -1;
        if (!GetConsoleMode(terminal_input, &saved_console_mode)) return -1;

        DWORD raw_mode = saved_console_mode;
        raw_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        if (!SetConsoleMode(terminal_input, raw_mode)) return -1;

        raw_mode_enabled = 1;
        return 0;
    }

    int TERM_disableRawMode(void) {
        if (!raw_mode_enabled) return 0;
        if (!SetConsoleMode(terminal_input, saved_console_mode)) return -1;

        raw_mode_enabled = 0;
        return 0;
    }

    int TERM_getch(void) {
        return _getch();
    }

    int TERM_getchNB(char *key) {
        if (key == NULL) return -1;
        if (!_kbhit()) return 0;
        *key = (char)_getch();
        return 1;
    }

#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <errno.h>

    static struct termios saved_terminal_mode;
    static int raw_mode_enabled;

    int TERM_enableRawMode(void) {
        if (raw_mode_enabled) return 0;
        if (tcgetattr(STDIN_FILENO, &saved_terminal_mode) == -1) return -1;

        struct termios raw_mode = saved_terminal_mode;
        raw_mode.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR |
                              IGNCR | ICRNL | IXON);
        raw_mode.c_oflag &= ~OPOST;
        raw_mode.c_cflag &= ~(CSIZE | PARENB);
        raw_mode.c_cflag |= CS8;
        raw_mode.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        raw_mode.c_cc[VMIN] = 1;
        raw_mode.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_mode) == -1) return -1;
        raw_mode_enabled = 1;
        return 0;
    }

    int TERM_disableRawMode(void) {
        if (!raw_mode_enabled) return 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &saved_terminal_mode) == -1) return -1;

        raw_mode_enabled = 0;
        return 0;
    }

    int TERM_getch(void) {
        int restore_mode = !raw_mode_enabled;
        if (restore_mode && TERM_enableRawMode() == -1) return EOF;

        unsigned char ch;
        ssize_t bytes;
        do {
            bytes = read(STDIN_FILENO, &ch, 1);
        } while (bytes == -1 && errno == EINTR);

        int result = bytes == 1 ? ch : EOF;
        if (restore_mode && TERM_disableRawMode() == -1) return EOF;
        return result;
    }

    int TERM_getchNB(char *key) {
        if (key == NULL) return -1;

        int restore_mode = !raw_mode_enabled;
        if (restore_mode && TERM_enableRawMode() == -1) return -1;

        fd_set readfds;
        struct timeval timeout = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        int result = 0;
        if (ready > 0) {
            ssize_t bytes = read(STDIN_FILENO, key, 1);
            result = bytes == 1 ? 1 : (bytes == 0 ? 0 : -1);
        } else if (ready < 0) {
            result = -1;
        }

        if (restore_mode && TERM_disableRawMode() == -1) return -1;
        return result;
    }

#endif