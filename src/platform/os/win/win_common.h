#pragma once

#ifdef _WIN32
#include <Windows.h>
#include "../../../common_c.h"

typedef struct {
    HINSTANCE reflib_library; // seems to be unused
    int reflib_active;        // also seems to be unused
    HWND hWnd;
    HINSTANCE hInstance;
    int activeApp;
    int isMinimized;
    int recenterMouse;
    uint sysMsgTime;
} WinVars_t;
#endif