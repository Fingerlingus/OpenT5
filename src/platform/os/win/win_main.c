#ifdef _WIN32

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <Windows.h>

#include "win_main.h"
#include "win_locale.h"
#include "win_common.h"
#include "../../../common_c.h"
#include "../../../FS/fs.h"
#include "../../../SEH/seh.h"
#include "../../../Sys/sys.h"
#include "../../../Sys/sys_critsect.h"
#include "../../../I/i_string.h"
#include "../../../Com/com.h"
#include "../../../Com/com_main.h"
#include "../../../PMem/pmem.h"

LPCSTR lpCaption_009d1ee0;
WinVars_t g_wv;

extern bool g_physicalMemoryInit;
extern bool s_nosnd;
extern char sys_cmdline[1024];
extern PhysicalMemory g_mem;

LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}


bool Win_RegisterClass(void) {
    WNDCLASSEXA windowClass = { sizeof(windowClass) };

    windowClass.lpfnWndProc   = (WNDPROC)MainWndProc;
    windowClass.hInstance     = g_wv.hInstance;
    windowClass.hIcon         = LoadIconA(g_wv.hInstance, (LPCSTR)0x1);
    windowClass.hCursor       = LoadCursorA(NULL,         (LPCSTR)0x7f00);
    windowClass.hbrBackground = CreateSolidBrush(0);
    windowClass.lpszClassName = "CoDBlackOps";
    if (RegisterClassExA(&windowClass) == 0)
        Com_Error(ERR_FATAL, "EXE_ERR_COULDNT_REGISTER_WINDOW");
    
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) 
{
    Com_Init("", false);
    Com_Printf(0, "Entered WinMain().");

    if (GetSystemMetrics(SM_REMOTESESSION) != 0) {
        MessageBoxA(
            NULL, "The game can not be run over a remote desktop connection.",
            lpCaption_009d1ee0, 0);

        return 0;
    }
    // TODO - minidump

    Com_Printf(0, "Initializing critical sections...");
    Sys_InitializeCriticalSections();
    //Win_InitMainThread();
    Com_Printf(0, "Initializing PMem...");
    PMem_Init();
    Com_Printf(0, "Initializing localization...");
    Win_InitLocalization();
    const char* s = "allowdupe";
    const size_t len = strlen(s);
    if (I_strnicmp(lpCmdLine, s, len) != 0 || lpCmdLine[9] > ' ') {
        if (strstr(lpCmdLine, "g_connectpaths 3") == NULL) {
            Com_Printf(0, "Getting semaphore file name...");
            Sys_GetSemaphoreFileName();
            if(Sys_CheckCrashOrRerun() == false) {
                goto done;
            }
        }
    }

    s_nosnd = (I_stristr(lpCmdLine, "nosnd") != NULL);

    if (hPrevInstance != NULL)
        goto done;

    Com_Printf(0, "Caching hInstance...");
    g_wv.hInstance = hInstance;
    Com_Printf(0, "Copying command line...");
    I_strncpyz(sys_cmdline, lpCmdLine, sizeof(sys_cmdline));
    Com_Printf(0, "Registering window class...");
    Win_RegisterClass();
    Com_Printf(0, "Setting error mode...");
    SetErrorMode(SEM_FAILCRITICALERRORS);
    Com_Printf(0, "Focusing window...");
    SetFocus(g_wv.hWnd);
    Com_Printf(0, "Entering Com_Main()...");
    //Com_Main();
    
done:
    Com_Printf(0, "Shutting down localization...");
    Win_ShutdownLocalization();
    Com_Printf(0, "Exiting...");
    return 0;
}

#endif // _WIN32