#include "../common_c.h"

#include "com_main.h"

#include "com.h"
#include "../FS/fs.h"
#include "../SEH/seh.h"
#include "../Sys/sys.h"
#include "../I/i_string.h"
#include "../Dvar/dvar.h"

extern bool g_allowMature;
extern char sys_cmdline[1024];
extern char g_open_automate_benchmark[1024];

int Com_Main() {
    g_allowMature = true;
    Com_InitParse();
    Dvar_Init();
    InitTiming();
    Sys_FindInfo();

    if (g_open_automate_benchmark[0] != '\0') {
        I_strncat(sys_cmdline, sizeof(sys_cmdline),
            " +set r_open_automate 1 +set com_introPlayed 1 +set ui_autoContinue 1 +devmap ");
        I_strncat(sys_cmdline, sizeof(sys_cmdline), g_open_automate_benchmark);
    }

    Sys_Milliseconds();
    Sys_SetupTLCallbacks(0x900000);
    //Com_Init("", false);
    PrintWorkingDir();

    while (true) {
        if (Sys_IsMinimized()) 
            Sys_SleepMs(5);
        
        Com_Frame();
        //FUN_005cc940();
        if (Dvar_GetBool("onlinegame")) {
            //PbProcessServerEvents();
        }
    }
}