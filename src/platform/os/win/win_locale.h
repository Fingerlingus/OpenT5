#pragma once

#ifdef _WIN32

#include "../../../common_c.h"

language_t Win_InitLocalization(void);
void Win_ShutdownLocalization(void);
char* Win_LocalizeRef(const char* str);

#endif // _WIN32