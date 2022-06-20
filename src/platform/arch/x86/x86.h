#pragma once

#include "../../../common_c.h"

#if IS_X86
#include <stdint.h>
void    cpuid(I_MachineWord regs[4], int type);
int64_t read_tsc(void);
#endif // IS_X86