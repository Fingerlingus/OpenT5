#include "../../../common_c.h"

#if IS_X86
#include <stdbool.h>

#if IS_MSVC
#   include <intrin.h>
#else
#   include <x86intrin.h>
#   include <cpuid.h>
#endif // IS_MSVC

#if defined _MSC_VER
void cpuid(I_MachineWord regs[4], int t) {
    __cpuidex(regs, t, 0);
}
#elif defined __GNUC__
void cpuid(int info[4], int InfoType) {
    __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}
#endif // defined _MSC_VER

#if defined _MSC_VER
int64_t read_tsc() {
    return __rdtsc();
}
#elif defined __GNUC__
int64_t read_tsc() {
    return __rdtsc();
}
#endif // defined _MSC_VER

#endif // IS_X86