#pragma once

int   I_strnicmp(const char* s1,  const char* s2,  int limit);
int   I_stricmp (const char* s1,  const char* s2);
char* I_strncpyz(char* dest,      const char* src, int size);
char* I_strncat (char* dest,      int count,       char* src);
char* I_stristr (const char* str, const char* substr);