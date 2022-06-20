#pragma once

#ifdef WIN32
#   include <Windows.h>
#endif // WIN32

#include <stdint.h>

#define _STR(x)     #x
#define  STR(x)     _STR(x)
#define  PRAGMA(x)  _Pragma(STR(x))
#define  MESSAGE(x) PRAGMA(message(x))

#ifdef _MSC_VER
#   define IS_MSVC 1
#endif // _MSC_VER

#ifdef __GNUC__
#   define IS_GCC_OR_CLANG 1
#endif // __GNUC__

#if IS_MSVC
#   if _WIN32 || _WIN64
#       define IS_X86 1
#       if _WIN64
#           define IS_AMD64
#           define COMPILE_64BIT 1
#       else // _WIN64
#           define IS_I386 1
#           define COMPILE_32BIT 1
#       endif // _WIN64
#   endif // _WIN32 || _WIN64
#elif IS_GCC_OR_CLANG // IS_MSVC
#   if __x86_64__
#       define IS_X86 1
#       define IS_AMD64 1
#       define COMPILE_64BIT 1
#   elif __i386__ // __x86_64__
#       define IS_X86 1 
#       define IS_I386 1
#       define COMPILE_32BIT 1
#   endif // __x86_64__
#endif // IS_MSVC 

#ifdef _POSIX_C_SOURCE
#   define POSIX_COMPLIANT 1
#endif // _POSIX_C_SOURCE

#if defined WIN32 && defined MAX_PATH
#   define OS_PATH_MAX MAX_PATH
#elif POSIX_COMPLIANT 1
#   include <limits.h>
#   ifdef PATH_MAX
#       define OS_PATH_MAX (PATH_MAX - 1)
#   else
#       define OS_PATH_MAX 255
        MESSAGE("OS is POSIX-compliant, but PATH_MAX is not defined; defaulting to 255.")
#   endif
#else
#   define OS_PATH_MAX 255
    MESSAGE("Unable to resolve maximum file path length for platform, defaulting to 255.")
#endif

#ifdef __linux__
#   define LINUX 1
#   define UNIX  1
#endif

#ifdef __APPLE__
#   define APPLE 1
#   define UNIX 1
#   include <TargetConditionals.h>
#   if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#       define APPLE_IOS 1
#   else
#       define APPLE_MACOS 1
#   endif
#endif

#ifdef __unix__ 
#   define UNIX 1
#endif

#if UNIX
#   include <sys/param.h>
//   sys/param.h includes macros for BSDs, so no need to declare them again
#endif

#if defined WIN32 && !CFG_RENDERER_FORCE_USE_VULKAN
#   define RENDERER_USE_D3D 1
#else
#   define RENDERER_USE_VULKAN 1
#endif


#define ARR_SIZE(a) sizeof((a)) / sizeof(*(a))

typedef unsigned char       uchar;
typedef unsigned int        uint;
typedef unsigned long       ulong;
typedef unsigned long long  ulonglong;
typedef uint_least32_t      I_uint32;
typedef int_least32_t       I_int;
typedef uint_least64_t      I_uint64;
typedef int_least64_t       I_int64;
#if COMPILE_32BIT
typedef uint32_t            I_MachineWord;
#elif COMPILE_64BIT // COMPILE_32BIT
typedef uint64_t            I_MachineWord;
#endif // COMPILE_32BIT

typedef enum {
    ERR_FATAL,
    ERR_DROP,
    ERR_SERVERDISCONNECT,
    ERR_DISCONNECT,
    ERR_SCRIPT,
    ERR_SCRIPT_DROP,
    ERR_LOCALIZATION
} errorParm_t;

typedef enum {
    LANGUAGE_ENGLISH,
    LANGUAGE_FRENCH,
    LANGUAGE_FRENCHCANADIAN,
    LANGUAGE_GERMAN,
    LANGUAGE_AUSTRIAN,
    LANGUAGE_ITALIAN,
    LANGUAGE_SPANISH,
    LANGUAGE_BRITISH,
    LANGUAGE_RUSSIAN,
    LANGUAGE_POLISH,
    LANGUAGE_KOREAN,
    LANGUAGE_JAPANESE,
    LANGUAGE_CZECH,
    MAX_LANGUAGES
} language_t;

typedef struct {
    char* language;
    char* strings;
} localization_t;

typedef struct {
    char* pszName;
    char* pszNameAbbr;
    int bPresent;
} languageInfo_t;

double  SecondsPerTick(void);
void    InitTiming(void);
void    PrintWorkingDir(void);
void    DoNothing();