#pragma once

#include <stdio.h>

#ifdef WIN32
#   define PATH_SEPARATOR (char)'\\'
#else 
#   define PATH_SEPARATOR (char)'/'
#endif // WIN32

typedef enum {
    OS_FOLDER_CONFIG,
    OS_FOLDER_DOCUMENTS,
    OS_FOLDER_DATA
} OsFolder;

FILE*  FS_FileOpenReadText(const char* path);
size_t FS_FileGetSize(FILE* f);
size_t FS_FileRead(void* buffer, size_t count, FILE* f);
int    FS_FileClose(FILE* f);

char* FS_GetOsFolderPath(OsFolder f, char* path);
int FS_CreatePath(const char* dir_path);