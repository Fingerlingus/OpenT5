#include <stdint.h>

#include "seh.h"

#include "../common_c.h"
#include "../I/i_string.h"

extern languageInfo_t g_languages[13];

#define SOME_PTR_CONSTANT 0xb7af58

int SEH_GetLanguageIndexForName(const char* lang, int* lang_index) {
    int idx = 0;
    for (languageInfo_t* languages_ptr = &g_languages[0]; (intptr_t)languages_ptr < SOME_PTR_CONSTANT; languages_ptr++) {
        if (I_stricmp(lang, languages_ptr->pszName) == 0) {
            *lang_index = idx;
            return 1;
        }
        idx = idx + 1;
    }

    *lang_index = 0;
    return 0;
}