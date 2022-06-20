#include "../common_c.h"

#if defined WIN32
#   include <Windows.h>
#elif APPLE || BSD || LINUX
#   include <sys/mman.h>
#else
#   include <stdlib.h>
#endif

#include <stdbool.h>
#include <string.h>

#include "pmem.h"

#include "../Com/com.h"

bool g_physicalMemoryInit = false;
PhysicalMemory g_mem;

#ifdef WIN32
void* PMem_Alloc(size_t size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}
#elif UNIX // WIN32
void* PMem_Alloc(size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);
}
#else // WIN32
void* PMem_Alloc(size_t size) {
    return malloc(size);
}
#endif // WIN32

void PMem_Init(void) {
    if (g_physicalMemoryInit == false) {
        g_physicalMemoryInit = true;

        const uint size = 0x12C00000;
        PMem_InitPhysicalMemory(&g_mem, "main", PMem_Alloc(size), size);
    }
}


void PMem_InitPhysicalMemory(PhysicalMemory* p, char* name, void* mem, uint size) {
    if (mem == NULL)
        Com_Error(ERR_FATAL, "Failed to initialize PMem.");

    memset(p, 0, sizeof(*p));
    p->name = name;
    p->buf = (uchar*)mem;
    p->prim[1].pos = size;
    p->size = size;
    return;
}