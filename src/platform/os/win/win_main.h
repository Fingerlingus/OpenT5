#pragma once

#ifdef WIN32
#include <Windows.h>

#include "../../../common_c.h"
#include "../../../PMem/pmem.h"

bool Win_RegisterClass(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow);

LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif // WIN32