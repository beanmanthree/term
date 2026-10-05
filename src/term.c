#include "term.h"

#include <stdio.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>

    char TERM_getch(void) {
        return _getch();
    }

    int TERM_getchNB(char *key) {
        if (key == NULL) return -1;
        if (!_kbhit()) return 0;
        *key = (char)_getch();
        return 1;
    }

    void TERM_sleepms(unsigned int ms) {
        Sleep(ms);
    }
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <errno.h>
    #include <time.h>

    char TERM_getch(void) {
        struct termios oldt, newt;
        char ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        if (read(STDIN_FILENO, &ch, 1) == -1) return EOF;
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

    int TERM_getchNB(char *key) {
        if (key == NULL) return -1;

        struct termios oldt, newt;
        fd_set readfds;
        struct timeval timeout = {0, 0};

        if (tcgetattr(STDIN_FILENO, &oldt) == -1) return -1;

        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 0;
        newt.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) return -1;

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

        if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1) return -1;
        return result;
    }

    void TERM_sleepms(unsigned int ms) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000L;
        while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
    }
#endif