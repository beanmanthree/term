#include "util.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <errno.h>
    #include <time.h>
#endif

int UTIL_clampInt(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

void UTIL_sleepms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
#endif
}