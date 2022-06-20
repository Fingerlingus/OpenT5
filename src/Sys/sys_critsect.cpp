#ifdef __cplusplus
#ifndef WIN32

#include <mutex>

#include "../common_c.h"
#include "sys_critsect.h"

std::mutex s_criticalSection[75];
bool bCriticalSectionsInitialized = false;

void Sys_InitializeCriticalSections(void) {
    for (int i = 0; i < ARR_SIZE(s_criticalSection); i++) 
        s_criticalSection[i].unlock();
}

extern "C" void Sys_EnterCriticalSection(CriticalSection sect) {
    if (bCriticalSectionsInitialized == false)
        return;

    s_criticalSection[sect].lock();
}

extern "C" void Sys_LeaveCriticalSection(CriticalSection sect) {
    if (bCriticalSectionsInitialized == false)
        return;

    s_criticalSection[sect].unlock();
}

#endif // WIN32
#endif // __cplusplus