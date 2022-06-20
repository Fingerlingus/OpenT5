#ifndef __cplusplus
#ifdef WIN32

#include <Windows.h>

#include <stdbool.h>

#include "../common_c.h"
#include "sys_critsect.h"

#include "../NET/net.h"

CRITICAL_SECTION s_criticalSection[75];
bool bCriticalSectionsInitialized = false;

void Sys_InitializeCriticalSections() {
    if (bCriticalSectionsInitialized == false) {
        bCriticalSectionsInitialized = true;
        //dFUN_00625500();
        for (int i = 0; i < ARR_SIZE(s_criticalSection); i++)
            InitializeCriticalSection(&s_criticalSection[i]);
    }
}

void Sys_EnterCriticalSection(CriticalSection sect) {
    if (bCriticalSectionsInitialized == false)
        return;

    EnterCriticalSection(&s_criticalSection[sect]);
}


void Sys_LeaveCriticalSection(CriticalSection sect) {
    if (bCriticalSectionsInitialized == false)
        return;

    LeaveCriticalSection(&s_criticalSection[sect]);
}

void Sys_LockWrite(FastCriticalSection* sect) {
    do {
        if (sect->readCount == 0) {
            if (sect->writeCount == 0)
                return;

            sect->writeCount--;
        }
        NET_Sleep(0);
    } while (true);
}

#endif // WIN32
#endif // __cplusplus