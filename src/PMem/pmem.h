#pragma once

#include "../common_c.h"

typedef struct {
    char* name;
    uint pos;
} PhysicalMemoryAllocation;

typedef struct {
    char* allocName;
    uint allocListCount;
    uint pos;
    PhysicalMemoryAllocation allocList[32];
    int memTrack;
} PhysicalMemoryPrim;

typedef struct {
    char* name;
    uchar* buf;
    PhysicalMemoryPrim prim[2];
    uint size;
} PhysicalMemory;

void PMem_Init(void);
void PMem_InitPhysicalMemory(PhysicalMemory* p, char* name, void* mem, uint size);