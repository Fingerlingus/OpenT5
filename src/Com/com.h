#pragma once

#include "../common_c.h"

#include <stdbool.h>

typedef enum {
    PARSE_TOKEN_UNKNOWN,
    PARSE_TOKEN_NUMBER,
    PARSE_TOKEN_STRING,
    PARSE_TOKEN_NAME,
    PARSE_TOKEN_HASH,
    PARSE_TOKEN_PUNCTUATION
} ParseTokenType;

typedef struct {
    char token[1024];
    ParseTokenType tokenType;
    int lines;
    bool ungetToken;
    bool spaceDelimited;
    bool keepStringQuotes;
    bool csv;
    bool negativeNumbers;
    char* errorPrefix;
    char* warningPrefix;
    int backup_lines;
    char* backup_text;
    char* parseFile;
} parseInfo_t;

typedef struct {
    parseInfo_t parseInfo[16];
    int parseInfoNum;
    char* tokenPos;
    char* prevTokenPos;
    char line[1024];
} ParseThreadInfo;

void Com_Error(errorParm_t err, const char* fmt, ...);
int  Com_sprintf(char* dest, size_t count, const char* fmt, ...);
void Com_ForceSafeMode(void);
void Com_InitParseInfo(parseInfo_t* p);
void Com_InitParse(void);
void Com_Init(const char* cmdline, bool b);
void Com_Printf(int i, const char* fmt, ...);
void Com_PrintWarning(int i, const char* fmt, ...);
void Com_PrintMessage(int i, const char* str, int j);
void Com_Frame(void);