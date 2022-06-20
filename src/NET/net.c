#include "../common_c.h"

#ifdef WIN32
#   include <Windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#   include <time.h>   // for nanosleep
#else
#   include <unistd.h> // for usleep
#endif // WIN32


#ifdef WIN32
void NET_Sleep(int ms) {
    Sleep(ms);
}
#elif _POSIX_C_SOURCE >= 199309L
void NET_Sleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
#else
void NET_Sleep(int ms) {
    if (ms >= 1000)
        sleep(ms / 1000);
    usleep((ms % 1000) * 1000);
}
#endif // WIN32