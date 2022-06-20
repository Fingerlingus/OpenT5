#include "../common_c.h"

#if defined WIN32
#   include <Windows.h>
#   include <intrin.h>
#   include <processthreadsapi.h>
#   include <shlobj_core.h>
#   include <TlHelp32.h>
#elif POSIX_COMPLIANT
#   include <pthread.h>
#   include <unistd.h>
#   include <fcntl.h>
#   if _POSIX_C_SOURCE >= 199309L
#       include <time.h>   // for nanosleep
#   endif // _POSIX_C_SOURCE >= 199309L
#endif // WIN32

#if UNIX
#   include <sys/sysinfo.h>
#endif // UNIX

#if RENDERER_USE_VULKAN
#   include <vulkan/vulkan.h>
#elif // RENDERER_USE_D3D
#   include <d3d9.h>
#endif // RENDERER_USE_VULKAN

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "sys_critsect.h"
#include "../FS/fs.h"
#include "../Sys/sys.h"
#include "../Com/com.h"
#include "../I/i_string.h"
#include "../platform/arch/x86/x86.h"

#ifdef WIN32
#   include "../CL/cl.h"
#   include "../platform/os/win/win_locale.h"
#   include "../platform/os/win/win_common.h"
#endif // WIN32

#define int3 uint_least32_t
#define int4 uint_least64_t
#define CONCAT31(a, b) (int3)(a) << 8  | (b)
#define CONCAT44(a, b) (int4)(a) << 32 | (b)
#define NO_RETURN _Noreturn

extern double msecPerRawTimerTick;
extern int g_com_error[15][16];
#ifdef WIN32
extern WinVars_t g_wv;
extern size_t _tls_index;
#endif

char sys_processSemaphoreFile[OS_PATH_MAX];
SysInfo sys_info;
char sys_cmdline[1024];
int sys_timeBase;
bool bRetrievedTimeBase = false;
uint s_cpuCount;
TlsData s_tlsData;
ulong threadId[15];

#ifdef WIN32
HANDLE g_hEventUnk1 = NULL;
tlSystemCallbacks tlCurSystemCallbacks;
HANDLE hThreads[15];
uint g_threadValues[70];
#endif // WIN 32

bool bExecutableNameAcquired = false;
char sys_executableName[OS_PATH_MAX];

#ifdef WIN32
uint32_t Sys_GetExecutableName(char* buffer) {
    DWORD res;
    if (bExecutableNameAcquired == false) {
        char current_process_path_buffer[OS_PATH_MAX]
        res = GetModuleFileNameA(NULL, current_process_path_buffer, sizeof(current_process_path_buffer));
        int pos = strlen(current_process_path_buffer);
        char* p = &current_process_path_buffer[pos - strlen(".exe")];
        char* exe_name_ptr = &current_process_path_buffer[0];
        while (p > &current_process_path_buffer[0]) {
            if ((*p == '\\') || (*p == ':') && (*p == '.')) {
                exe_name_ptr = p + 1;
                current_process_path_buffer[pos - strlen(".exe")] = '\0';
                break;
            }
            p--;
        }
        I_strncpyz(sys_executableName, exe_name_ptr, sizeof(sys_executableName));
    }

    I_strncpyz(buffer, sys_executableName, OS_PATH_MAX);
    return (uint32_t)res;
}
#else
uint32_t Sys_GetExecutableName(char* buffer) {
    if (bExecutableNameAcquired == false)
        return -1;

    // no need to acquire sys_executableName, should already have been placed
    // should have already been copied from argv[0]
    I_strncpyz(buffer, sys_executableName, OS_PATH_MAX);
    return 0;
}
#endif // WIN32

void Sys_GetSemaphoreFileName(void) {
    char appdata_path_buffer[OS_PATH_MAX];
    char current_process_path_buffer[OS_PATH_MAX];

    Sys_GetExecutableName(current_process_path_buffer);
    char* exe_name_ptr = &current_process_path_buffer[0];
    
    FS_GetOsFolderPath(OS_FOLDER_DATA, appdata_path_buffer);
    int len = strnlen(appdata_path_buffer, OS_PATH_MAX);
    char path_separator[] = { PATH_SEPARATOR };
    if (appdata_path_buffer[len - 1] != PATH_SEPARATOR) 
        I_strncat(appdata_path_buffer, OS_PATH_MAX, path_separator);
    
    FS_CreatePath(appdata_path_buffer);
#ifdef WIN32
    const char* fmt = "%s__%s"; // hidden file is an attribute on Windows
                                // no need to prefix with '.' 
                                // like in Unix-like OSes
#else
    const char* fmt = "%s.__%s";
#endif // WIN32
    Com_sprintf(sys_processSemaphoreFile, OS_PATH_MAX, fmt, appdata_path_buffer, exe_name_ptr);
    return;
}

#if defined WIN32
bool Sys_IsGameProcess(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) 
        return false;
    
    CloseHandle(hProcess);

    if (CreateToolhelp32Snapshot(PROCESS_VM_OPERATION, pid) == (HANDLE)-1)
        return true;

    MODULEENTRY32 me = { sizeof(me) };
    if (Module32First(hProcess, &me) == 0)
        return false;

    char file_name_buffer[MAX_PATH];
    GetModuleFileNameA(NULL, file_name_buffer, MAX_PATH);
    const char* read_ptr = file_name_buffer;
    char* exe_name_ptr = (char*)read_ptr;


    while (*read_ptr != '\0') {
        if ((*read_ptr == PATH_SEPARATOR) || (*read_ptr == ':')) 
            exe_name_ptr = (char*)read_ptr + 1;
               
        read_ptr = read_ptr + 1;
    }
            
    while (I_stricmp(me.szModule, exe_name_ptr) != 0) {
        if (Module32Next(hProcess, &me) == FALSE) {
            CloseHandle(hProcess);
            return false;
        }
    }

    CloseHandle(hProcess);
    return true;
}
#elif LINUX // WIN32
bool Sys_IsGameProcess(int pid) {
    char symlink_path[OS_PATH_MAX];
    char executable_path[OS_PATH_MAX];

    snprintf(symlink_path, sizeof(symlink_path), "/proc/%d/exe", pid);
    ssize_t res = readlink(symlink_path, executable_path, sizeof(executable_path));
    if (res < 0)
        Com_Error(ERR_FATAL, "Sys_IsGameProcess(): readlink() failed.");
    
    if(res < sizeof(executable_path))
        executable_path[res] = '\0';

    const char* executable_name = NULL;
    for (int i = res; i > 0; --i) {
        if (executable_path[i] == PATH_SEPARATOR) {
            executable_name = &executable_path[i + 1];
            break;
        }
    }

    if(executable_path == NULL)
        Com_Error(ERR_FATAL, "Sys_IsGameProcess(): failed to get executable name.");

    if (I_stricmp(executable_name, sys_executableName) == 0)
        return true;

    return false;
}
#endif // defined WIN32

#ifdef WIN32
uintptr_t DoSetEvent_UNK(void) {
    uint uVar1;

    if (g_hEventUnk1 != NULL) {
        g_hEventUnk1 = (HANDLE)SetEvent(g_hEventUnk1);
    }
    uVar1 = (uintptr_t)g_hEventUnk1 >> 8;
    g_hEventUnk1 = NULL;
    return CONCAT31((int3)uVar1, 1);
}
#endif // WIN32


NO_RETURN void Sys_NoFreeFilesError(void) {
    Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
// TODO - implement message box like Windows build
#ifdef WIN32
    LPCSTR lpCaption = Win_LocalizeRef("WIN_DISK_FULL_TITLE");
    LPCSTR lpText    = Win_LocalizeRef("WIN_DISK_FULL_BODY");
    HWND hWnd        = GetActiveWindow();
    MessageBoxA(hWnd, lpText, lpCaption, MB_ICONERROR);
    DoSetEvent_UNK();
#endif // WIN32
    _exit(-1);
}


#if defined WIN32
I_MachineWord Sys_GetCurrentPid(void) {
    return (I_MachineWord)GetCurrentProcessId();
}
#elif POSIX_COMPLIANT
I_MachineWord Sys_GetCurrentPid(void) {
    return (I_MachineWord)getpid();
}
#endif // defined WIN32

#ifdef WIN32
bool Sys_CheckCrashOrRerun(void) {
    if (sys_processSemaphoreFile[0] == '\0')
        return true;

    DWORD currentProcessId = (DWORD)Sys_GetCurrentPid();
    //FUN_009a2630();
    HANDLE hFile = CreateFileA(sys_processSemaphoreFile, GENERIC_READ, 0, 
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL);

    if (hFile != (HANDLE)-1) {
        DWORD fileBuffer, nBytesRead;
        if (ReadFile(hFile, &fileBuffer, sizeof(fileBuffer),
                     &nBytesRead, NULL) == FALSE || nBytesRead != 4) 
        {
            CloseHandle(hFile);
        }

        else {
            CloseHandle(hFile);
            if ((currentProcessId != fileBuffer) && Sys_IsGameProcess(fileBuffer) == true)
                return false;
            
            LPCSTR lpCaption = Win_LocalizeRef("WIN_IMPROPER_QUIT_TITLE");
            LPCSTR lpText    = Win_LocalizeRef("WIN_IMPROPER_QUIT_BODY");
            HWND hWnd        = GetActiveWindow();
            switch (MessageBoxA(hWnd, lpText, lpCaption, MB_YESNOCANCEL | MB_ICONEXCLAMATION)) {
            case IDYES:
                Com_ForceSafeMode();
                break;
            case IDNO:
            case IDCANCEL:
                return false;
            default:
                ;
            }
        }
    }
    
    hFile = CreateFileA(sys_processSemaphoreFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);

    if (hFile == (HANDLE)-1) 
        Sys_NoFreeFilesError();

    DWORD nBytesRead;
    if ((WriteFile(hFile, &currentProcessId, sizeof(currentProcessId), &nBytesRead, NULL) != FALSE) &&
        (nBytesRead == 4)) 
    {
        CloseHandle(hFile);
        return true;
    }
    CloseHandle(hFile);
    Sys_NoFreeFilesError();
}
#elif POSIX_COMPLIANT // WIN32
bool Sys_CheckCrashOrRerun(void) {
    if (sys_processSemaphoreFile[0] == '\0')
        return true;

    pid_t currentProcessId = (pid_t)Sys_GetCurrentPid();
    //FUN_009a2630();
    int fd = open(sys_processSemaphoreFile, O_RDONLY, 0666);
    if (fd >= 0) {
        int bytes;
        if (read(fd, &bytes, sizeof(bytes)) != sizeof(bytes))
            close(fd);
        else {
            close(fd);
            if ((currentProcessId != bytes) && Sys_IsGameProcess(bytes) == true)
                return false;
            // TODO - implement message-box like in Windows build
        }
    }

    fd = open(sys_processSemaphoreFile, O_WRONLY | O_CREAT, 0666);
    if (fd < 0)
        Sys_NoFreeFilesError();

    if (write(fd, &currentProcessId, sizeof(currentProcessId)) != sizeof(currentProcessId)) {
        close(fd);
        return true;
    }

    close(fd);
    Sys_NoFreeFilesError();
}
#endif // WIN32

#ifdef WIN32
void Sys_DeleteFile(const char* path) {
    DeleteFileA(path);
}
#else
void Sys_DeleteFile(const char* path) {
    remove(path);
}
#endif // WIN32

void Sys_NormalExit(void) {
    Sys_DeleteFile(sys_processSemaphoreFile);
}

uint Sys_GetCpuCount(void) {
    return s_cpuCount;
}


#ifdef WIN32
int Sys_SystemMemoryMB(void) {
    HMODULE hModule = GetModuleHandleA("kernel32.dll");
    FARPROC f = GetProcAddress(hModule, "GlobalMemoryStatusEx");
    if (hModule == NULL || f == NULL) {
        MEMORYSTATUS mem_status = { sizeof(mem_status) };
        GlobalMemoryStatus(&mem_status);
        if (mem_status.dwAvailVirtual < 0x8000000) {
            LPCSTR lpCaption = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
            LPCSTR lpText = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
            HWND hWnd = GetActiveWindow();
            if (MessageBoxA(hWnd, lpText, lpCaption, MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
                Sys_NormalExit();
                exit(0);
            }
        }
        int mem_in_megabytes = mem_status.dwTotalPhys;
        if (mem_in_megabytes > 1024)
            mem_in_megabytes = 1024;
        return mem_in_megabytes;
    }

    MEMORYSTATUSEX mem_status = { sizeof(mem_status) };
    f(&mem_status);
    if (mem_status.ullAvailVirtual < 0x8000000) {
        LPCSTR lpCaption = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
        LPCSTR lpText = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
        HWND hWnd = GetActiveWindow();
        if (MessageBoxA(hWnd, lpText, lpCaption, MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
            Sys_NormalExit();
            exit(0);
        }
    }
    ulonglong mem_in_megabytes = mem_status.ullTotalPhys;
#   if defined COMPAT_BUILD && COMPAT_BUILD <= COMPAT_WEAK
    if (mem_in_megabytes > 1024)
        mem_in_megabytes = 1024;
#   endif
    return mem_in_megabytes;
}
// TODO - add low memory alert like in Windows build
#elif UNIX // WIN32
int Sys_SystemMemoryMB(void) {
    struct sysinfo s;
    if (sysinfo(&s) < 0)
        return 0;

    int mem_in_megabytes = (int)(s.totalram / (1024 * 1024));

#   if defined COMPAT_BUILD && COMPAT_BUILD <= COMPAT_WEAK
    if (mem_in_megabytes > 1024)
        mem_in_megabytes = 1024;
#   endif
    return mem_in_megabytes;
}
#endif // WIN32

#if RENDERER_USE_VULKAN
VkInstance Sys_CreateVulkanInstance(const char* name) {
    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType             = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo  = &appInfo;
    createInfo.enabledLayerCount = 0;

    VkInstance instance;
    VkResult res = vkCreateInstance(&createInfo, NULL, &instance);
    if (res != VK_SUCCESS)
        Com_Error(ERR_FATAL, "vkCreateInstance() failed with code %d.", res);

    return instance;
}
#endif // RENDERER_USE_VULKAN

#if RENDERER_USE_VULKAN
void Sys_DetectVideoCard(char* buffer, size_t size) {
    const char* desc = "Unknown device card";
    I_strncpyz(buffer, desc, size);

    VkInstance instance = Sys_CreateVulkanInstance("Get Description");
    
    uint32_t deviceCount = 0;
    VkResult res = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (res != VK_SUCCESS)
        Com_Error(ERR_FATAL, "vkEnumeratePhysicalDevices() failed with code %d.", res);

    if (deviceCount == 0)
        Com_Error(ERR_FATAL, "vkEnumeratePhysicalDevices() failed to enumerate any compatible devices.");

    VkPhysicalDevice devices[4];
    res = vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    if (res != VK_SUCCESS || res != VK_INCOMPLETE)
        Com_Error(ERR_FATAL, "vkEnumeratePhysicalDevices() failed with code %d.", res);

    int n = deviceCount > ARR_SIZE(devices) ? ARR_SIZE(devices) : deviceCount;
    int choice = 0;
    if (deviceCount > 1)
        Com_PrintWarning(0, "vkEnumeratePhysicalDevices() returned more than 1 device, attempting to find best device.");
    
    VkPhysicalDeviceProperties deviceProperties;
    for (int i = 0; i < n; i++) {  
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            break;
    }

    I_strncpyz(buffer, deviceProperties.deviceName, size);
    vkDestroyInstance(instance, NULL);
}
#else 
void Sys_DetectVideoCard(char* buffer, size_t size) {
    const char* desc = "Unknown device card";
    I_strncpyz(buffer, desc, size);

    IDirect3D9* d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d == NULL)
        return;

    D3DADAPTER_IDENTIFIER9 id;
    if (d->lpVtbl->GetAdapterIdentifier(d, 0, 0, &id) == D3D_OK) {
#if !defined COMPAT_BUILD || COMPAT_BUILD != COMPAT_FULL
        memset(buffer, 0, strlen(desc) + 1);
#endif // !defined COMPAT_BUILD || COMPAT_BUILD == COMPAT_FULL
        I_strncpyz(buffer, id.Description, size);
    }

    d->lpVtbl->Release(d);
}
#endif // RENDERER_USE_VULKAN

bool Sys_SupportsSSE(void) {
    I_MachineWord regs[4] = { 0, 0, 0, 0 };
    cpuid(regs, 1);
    return (regs[3] & (1 << 25)) != 0;
}

// not how the actual engine does it, but this works and it's easier
#if !defined COMPAT_BUILD || COMPAT_BUILD != COMPAT_FULL
void Sys_DetectCpuVendorAndName(char* cpu_vendor, char* cpu_name) {
    cpuid((I_MachineWord*)cpu_vendor, 0);
    cpu_vendor[12] = '\0';

    for (int i = 0; i < 3; i++) 
        cpuid((I_MachineWord*)(cpu_name + (16 * i)), 0x80000002 + i);
    cpu_name[48] = '\0';
}
#endif // !defined COMPAT_BUILD || COMPAT_BUILD == COMPAT_FULL

// TODO - clean up
double Sys_BenchmarkGHz(void) {
    uint uVar3;

#if defined WIN32
    HANDLE hThread = GetCurrentThread();
    int nPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
#elif POSIX_COMPLIANT
    int policy, max_policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(max_policy);
    pthread_setschedparam(pthread_self(), max_policy, &param);
#endif // defined WIN32

    uint uVar6 = 0xffffffff;
    uint uVar5 = 0xffffffff;
    for(int i = 1000; i > 0; i--) {
        Sys_SleepMs(0);
        int64_t tsc_count = read_tsc();
        uVar3 = 0;
        do {
            uVar3 = uVar3 + 1;
        } while (uVar3 < 1000);
        int64_t tsc_count_2 = read_tsc();
        uint tsc_count_elapsed = (uint)tsc_count_2 - (uint)tsc_count;
        uVar3 = ((int)((ulonglong)tsc_count_2 >> 0x20) - (int)((ulonglong)tsc_count >> 0x20)) -
                (uint)((uint)tsc_count_2 < (uint)tsc_count);
        if ((uVar3 <= uVar5) && ((uVar3 < uVar5 || (tsc_count_elapsed < uVar6)))) {
            uVar5 = uVar3;
            uVar6 = tsc_count_elapsed;
        }
    }

#if defined WIN32
    SetThreadPriority(hThread, nPriority);
#elif POSIX_COMPLIANT
    pthread_setschedparam(pthread_self(), policy, &param);
#endif

    return 0.1010328 /
           ((-(double)(((ulonglong)uVar5 & 0x80000000) << 0x20) +
           (double)(CONCAT44(uVar5, uVar6) & ~(1ull << 63))) * msecPerRawTimerTick);
}

// TODO - add unknown functions
#if !defined COMPAT_BUILD || COMPAT_BUILD != COMPAT_FULL
void Sys_SetAutoConfigureGHz(SysInfo* p) {
    double benchmark = Sys_BenchmarkGHz();
    double factor = 1.0;
    if (p->physicalCpuCount == 2)
        factor = 1.75;
    else if (p->physicalCpuCount > 2)
        factor = 2.0;
    p->configureGHz = benchmark * factor;
}
#endif // !defined COMPAT_BUILD || COMPAT_BUILD == COMPAT_FULL

void Sys_FindInfo(void) {
    sys_info.logicalCpuCount = Sys_GetCpuCount();
    sys_info.cpuGHz = 1.0 / (msecPerRawTimerTick * 1.0 * 1000000.0);
    sys_info.sysMB = Sys_SystemMemoryMB();
    Sys_DetectVideoCard(sys_info.gpuDescription, sizeof(sys_info.gpuDescription));
    sys_info.SSE = Sys_SupportsSSE();
    Sys_DetectCpuVendorAndName(sys_info.cpuVendor, sys_info.cpuName);
    Sys_SetAutoConfigureGHz(&sys_info);
}

#ifdef WIN32
int Sys_Milliseconds(void) {
    if (bRetrievedTimeBase == false) {
        sys_timeBase = timeGetTime();
        bRetrievedTimeBase = true;
    }

    return timeGetTime() - sys_timeBase;
}
#elif UNIX // WIN32
int Sys_Milliseconds(void) {
    struct sysinfo s;
    int time;
    if (sysinfo(&s) < 0)
        time = -1;
    else
        time = (int)s.uptime * 1000; // time is supplied in seconds, needs to be ms
    if (bRetrievedTimeBase == false) {   
        sys_timeBase = time;
        bRetrievedTimeBase = true;
    }

    return time - sys_timeBase;
}
#endif // WIN32

// Windows-specific code, no need to port
#ifdef WIN32
void tlSetSystemCallbacks(tlSystemCallbacks* callbacks) {
    tlSystemCallbacks* curCallbacks = &tlCurSystemCallbacks;
    for (int i = 8; i != 0; --i) {
        curCallbacks->ReadFile = callbacks->ReadFile;
        callbacks              = (tlSystemCallbacks*)&callbacks->ReleaseFile;
        curCallbacks           = (tlSystemCallbacks*)&curCallbacks->ReleaseFile;
    }
    return;
}
// TODO - implement functions
void Sys_SetupTLCallbacks(int size) {
    tlSystemCallbacks systemCallbacks;

    systemCallbacks.ReadFile      = NULL;
    systemCallbacks.ReleaseFile   = NULL;
    systemCallbacks.CriticalError = &DoNothing;
    systemCallbacks.Warning       = NULL;
    systemCallbacks.DebugPrint    = NULL;
    systemCallbacks.MemAlloc      = NULL;
    systemCallbacks.MemRealloc    = CL_GetFirstActiveLocalClient;
    systemCallbacks.MemFree       = NULL;
    tlSetSystemCallbacks(&systemCallbacks);
}
#endif // WIN32

#ifdef WIN32
bool Sys_IsMinimized(void) {
    return g_wv.isMinimized != 0;
}
#endif // WIN32

#ifdef WIN32
void Sys_SleepMs(int milliseconds) {
    Sleep(milliseconds);
}
#elif _POSIX_C_SOURCE >= 199309L
void Sys_SleepMs(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
#else
void Sys_SleepMs(int milliseconds) {
    if (milliseconds >= 1000)
        sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
}
#endif // WIN32

#ifdef WIN32
I_MachineWord Sys_GetCurrentThreadId(void) {
    return (I_MachineWord)GetCurrentThreadId();
}
#endif

void Sys_SetValue(int idx, void* ptr) {
    TlsData* data = (TlsData*)TlsGetValue(_tls_index * 4);
    data->threadValues[idx] = ptr;
}

void Com_InitThreadData(int threadId) {
    Sys_SetValue(1, (void*)((uintptr_t)g_com_error + threadId * 0x1004 + 0x3a0));
    Sys_SetValue(2, g_com_error[threadId]);
    Sys_SetValue(3, g_traceThreadInfo[threadId]);
    if (threadId == 1) {
        Sys_SetValue(4, g_cmd_args[1]);
        return;
    }
    Sys_SetValue(4, g_cmd_args[0]);
}

void Sys_InitMainThread(void) {
    TlsData* data = (TlsData*)TlsGetValue(_tls_index * 4);
    if (data->currentThreadId == 0)
        data->currentThreadId = Sys_GetCurrentThreadId();

    threadId[0] = data->currentThreadId;

    HANDLE hSourceProcessHandle = GetCurrentProcess();
    HANDLE hSourceHandle = GetCurrentThread();

    DuplicateHandle(hSourceProcessHandle, hSourceHandle, hSourceProcessHandle, hThreads, 0, FALSE, 2);
    Win_InitThreads();
    data->threadValues = (void*)&g_threadValues;

    FUN_0041ceb0();
    Com_InitThreadData(0);
}