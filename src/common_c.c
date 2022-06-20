#include "common_c.h"

#if defined WIN32
#   include <Windows.h>
#   include <intrin.h>
#   include <direct.h>
#elif POSIX_COMPLIANT // WIN32
#   include <unistd.h>
#endif // WIN32

#include <stdbool.h>
#include <stdint.h>

#include "Com/com.h"

bool g_allowMature;
bool s_nosnd;
localization_t localization;
char language_buffer[4096];
languageInfo_t g_languages[13];
double msecPerRawTimerTick;

#ifdef WIN32
char   g_open_automate_benchmark[OS_PATH_MAX];
size_t _tls_index;
#endif // WIN32

#ifdef WIN32
double SecondsPerTick() {
    long long l[2];

    QueryPerformanceFrequency((void*)l);
    return 1.0 / (double)l[0];
}
// TODO - get actual tick counter
#else
double SecondsPerTick() {
    return 1 / 1'000'0000;
}
#endif // WIN32

void InitTiming(void) {
    msecPerRawTimerTick = SecondsPerTick() * 1000.0;
    return;
}

#if defined WIN32
void PrintWorkingDir(void) {
    char path[OS_PATH_MAX];
    _getcwd(path, sizeof(path));
    Com_Printf(0x10, "Working directory: %s\n");
}
#elif POSIX_COMPLIANT
void PrintWorkingDir(void) {
    char path[OS_PATH_MAX];
    getcwd(path, sizeof(path));
    Com_Printf(0x10, "Working directory: %s\n");
}

void DoNothing() {

}
#endif // defined WIN32