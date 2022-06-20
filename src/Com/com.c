#ifdef _WIN32
#   define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "com.h"
#include "../Sys/sys_critsect.h"
#include "../common_c.h"

int com_safemode = 0;
ParseThreadInfo g_parse[4];
int g_com_error[15][16];

void Com_ForceSafeMode(void) {
    com_safemode = 1;
}

void Com_Error(errorParm_t err, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (err == ERR_FATAL)
        exit(-1);
}

int Com_sprintf(char* dest, size_t count, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(dest, count, fmt, args);
    dest[count - 1] = '\0';
    return r;
}

void Com_InitParseInfo(parseInfo_t* p) {
    p->lines = 1;
    p->ungetToken = false;
    p->spaceDelimited = true;
    p->keepStringQuotes = false;
    p->csv = false;
    p->negativeNumbers = false;
    p->errorPrefix = "";
    p->warningPrefix = "";
    p->backup_lines = 0;
    p->backup_text = NULL;
}

void Com_InitParse(void) {
    for(int i = 0; i < ARR_SIZE(g_parse); i++)
        Com_InitParseInfo(g_parse[i].parseInfo);
}

// TODO
void Com_Init(const char* cmdline, bool b) {
    fopen("log.txt", "w");
}

void Com_Printf(int i, const char* fmt, ...) {
    if (i < 0x1f) {
        char buffer[4092];
        va_list va;
        va_start(va, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, va);
        Com_PrintMessage(i, buffer, 0);
        va_end(va);
    }
}

// TODO
void Com_PrintMessage(int i, const char* str, int j) {
    Sys_EnterCriticalSection(CRITSECT_CONSOLE);
    FILE* f = fopen("log.txt", "a");
    fprintf(f, "%s\n", str);
    Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
}

void Com_PrintWarning(int i, const char* fmt, ...) {
    char buffer[4092];
    if (i < 0x1f) {
        va_list va;
        va_start(va, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, va);
        Com_PrintMessage(i, buffer, 0);
        va_end(va);
    }
}

void Com_Frame(void) {

}