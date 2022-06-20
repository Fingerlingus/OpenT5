#pragma once

#ifdef WIN32
#   include <Windows.h>
#endif

#include "../common_c.h"

#include <stdbool.h>

typedef struct {
    double cpuGHz;
    double configureGHz;
    int logicalCpuCount;
    int physicalCpuCount;
    int sysMB;
    char gpuDescription[512];
    bool SSE;
    char cpuVendor[13];
    char cpuName[49];
} SysInfo;

#ifdef WIN32
typedef struct {
    uchar* Buf;
    uint   Size;
    uint   UserData;
} tlFileBuf;

typedef struct {
    bool  (*ReadFile)     (char*, tlFileBuf*, uint, uint*);
    void  (*ReleaseFile)  (tlFileBuf*);
    void  (*CriticalError)(char*);
    void  (*Warning)      (char*);
    void  (*DebugPrint)   (char*);
    void* (*MemAlloc)     (uint,  uint,       uint*);
    void* (*MemRealloc)   (void*, uint,       uint, uint*);
    void  (*MemFree)      (void*);
} tlSystemCallbacks;
#endif

typedef struct {
    uchar  unk[0x28];
    void** threadValues;
    I_MachineWord currentThreadId;
} TlsData;

void Sys_GetSemaphoreFileName(void);
bool Sys_CheckCrashOrRerun(void);
uint Sys_GetCpuCount(void);
int  Sys_SystemMemoryMB(void);
void Sys_DetectVideoCard(char* buffer, size_t size);
bool Sys_SupportsSSE(void);
void Sys_DetectCpuVendorAndName(char* cpu_vendor, char* cpu_name);
void Sys_SetAutoConfigureGHz(SysInfo* p);
void Sys_FindInfo(void);
int  Sys_Milliseconds(void);
void Sys_SetupTLCallbacks(int size);
bool Sys_IsMinimized(void);
void Sys_SleepMs(int milliseconds);