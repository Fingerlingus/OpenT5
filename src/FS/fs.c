#include "../common_c.h"

#if defined WIN32
#   define _CRT_SECURE_NO_WARNINGS 1
#   include <Windows.h>
#   include <shlobj_core.h>
#elif UNIX // WIN32
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

#include <stdlib.h>
#include <stdbool.h>

#include "fs.h"

#include "../Com/com.h"
#include "../I/i_string.h"

FILE* FS_FileOpenReadText(const char* path) {
    return fopen(path, "rt");
}

size_t FS_FileGetSize(FILE* f) {
    long offset = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, offset, SEEK_SET);
    return size;
}

size_t FS_FileRead(void* buffer, size_t count, FILE* f) {
    return fread(buffer, 1, count, f);
}

int FS_FileClose(FILE* f) {
    return fclose(f);
}

#if WIN32
char* FS_GetOsFolderPath(OsFolder f, char* buffer) {
    if (buffer == NULL) {
        Com_PrintWarning(0, "FS_GetOsFolderPath: buffer is NULL.");
        return NULL;
    }

    int csidl;
    switch (f) {
    case OS_FOLDER_DATA:
    case OS_FOLDER_CONFIG:
        csidl = CSIDL_APPDATA;
        break;
    case OS_FOLDER_DOCUMENTS:
        csidl = CSIDL_MYDOCUMENTS;
    default:
        Com_PrintWarning(0, "FS_GetOsFolderPath: invalid folder given.");
        return NULL;
    }

    CHAR path_buffer[1028];
    SHGetFolderPathA(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, path_buffer);
    I_strncpyz(buffer, path_buffer, 256);
    I_strncat(buffer, 256, "\\Activision\\CoD");
    return buffer;
}
#else
char* FS_GetOsFolderPath(OsFolder f, char* buffer) {
    if (buffer == NULL) {
        Com_PrintWarning(0, "FS_GetOsFolderPath: buffer is NULL.");
        return NULL;
    }

    char* path;
    switch (f) {
    case OS_FOLDER_DATA:
        path = getenv("XDG_DATA_HOME");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            break;
        }

        path = getenv("HOME");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            I_strncat(buffer, OS_PATH_MAX, "/.local/share");
            break;
        }
        I_strncpyz(buffer, "/usr/share", OS_PATH_MAX);
        break;
    case OS_FOLDER_CONFIG:
        path = getenv("XDG_CONFIG_HOME");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            break;
        }

        path = getenv("HOME");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            I_strncat(buffer, OS_PATH_MAX, "/.config");
            break;
        }
        I_strncpyz(buffer, "/etc", OS_PATH_MAX);
        break;
    case OS_FOLDER_DOCUMENTS:
        path = getenv("XDG_DOCUMENTS_DIR");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            break;
        }

        path = getenv("HOME");
        if (path != NULL) {
            I_strncpyz(buffer, path, OS_PATH_MAX);
            I_strncat(buffer, OS_PATH_MAX, "/Documents");
            break;
        }
        // Intentional fallthrough
    default:
        Com_PrintWarning(0, "FS_GetOsFolderPath: invalid folder given.");
        return NULL;
    }

    I_strncat(buffer, 256, "/Activision/CoD");
}
#endif // WIN32


#if defined WIN32
int FS_CreatePath(const char* dir_path) {
#   ifndef COMPAT_BUILD || COMPAT_BUILD != COMPAT_FULL
    if (strstr(dir_path, "..") != NULL || strstr(dir_path, "::") != NULL || strstr(dir_path, ".\\") != NULL) {
#   else
    if (strstr(dir_path, "..") != NULL || strstr(dir_path, "::") != NULL) {
#   endif // COMPAT_BUILD || COMPAT_BUILD != COMPAT_FULL
        Com_PrintWarning(10, 
                         "WARNING: refusing to create relative path \"%s\"\n",
                         dir_path);
        return 1;
    }

    char buf[OS_PATH_MAX];
    I_strncpyz(buf, dir_path, sizeof(buf));
    
    
    for(char* p = &buf[2]; *p != '\0'; p++) {
        if (*p == PATH_SEPARATOR) {
            *p = '\0';
            if (CreateDirectoryA(buf, NULL) == FALSE) {
                switch (GetLastError()) {
                case ERROR_PATH_NOT_FOUND:
                    return -1;

                case ERROR_ALREADY_EXISTS:
                default:
                    break;
                }
            }
            
            *p = '\\';
        }
    }
    return 0;
}
#elif UNIX
int FS_CreatePath(const char* dir_path) {
    if (strstr(dir_path, "..") != NULL || strstr(dir_path, "./") != NULL) {
        Com_PrintWarning(10,
            "WARNING: refusing to create relative path \"%s\"\n",
            dir_path);
        return 1;
    }

    char buf[OS_PATH_MAX];
    I_strncpyz(buf, dir_path, sizeof(buf));

    for (char* p = &buf[2]; *p != '\0'; p++) {
        if (*p == PATH_SEPARATOR) {
            *p = '\0';
            if (mkdir(buf, S_IRWXU) < 0)
                return -1;

            *p = '\\';
        }
    }
    return 0;
}
#endif // WIN32