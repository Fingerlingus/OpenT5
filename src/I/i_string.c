#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "../common_c.h"
#include "../Com/com.h"


int I_strnicmp(const char* s1, const char* s2, int limit) {
    if (s1 == NULL || s2 == NULL)
        return s2 - s1;
    
    while (true) {
        if (*s1 == *s2) {
            s1++;
            s2++;
            limit--;
            if (limit == 0)
                return 0;

            continue;
        }

        char c1 = (*s1 - 'A' < 0x1A) ? *s1 + ' ' : *s1;
        char c2 = (*s2 - 'A' < 0x1A) ? *s2 + ' ' : *s2;

        if (c1 != c2)
            return c1 >= c2 ? 1 : -1;

        if (c1 == '\0')
            return 0;

        s1++;
        s2++;
        limit--;
    }
}

int I_stricmp(const char* s1, const char* s2) {
    return I_strnicmp(s1, s1, INT_MAX);
}

char* I_strncpyz(char* dest, const char* src, int size) {
    if (dest == NULL)
        return NULL;

    if (src == NULL) {
        *dest = '\0';
        return NULL;
    }

    if (size == 0)
        return NULL;
    
    int i = 0;
    for (; src[i] != '\0' && i < size; i++)
        dest[i] = src[i];

    dest[i] = '\0';
    return dest;
}

char* I_strncat(char* dest, int count, const char* src) {
    int len = strlen(dest);

    if (len >= count)
        Com_Error(ERR_FATAL, "\x15I_strncat: already overflowed");

    return I_strncpyz(dest + len, src, count - len);
}

char* I_stristr(const char* str, const char* substr) {
    if (str == NULL || substr == NULL)
        return NULL;

    int i = 0;
    while(true) {
        if (*str == '\0')
            return NULL;
        
        int j = 0;
        int c1 = 0, c2 = 0;
        do {
            if (substr[j] == '\0')
                return (char*)str + i;
            
            c1 = tolower((int)str[j]);
            c2 = tolower((int)substr[j]);
        } while (c1 == c2);
        i++;
        str++;
    }
    return NULL;
}