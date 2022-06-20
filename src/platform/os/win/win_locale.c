#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>

#include "win_locale.h"

#include "../../../FS/fs.h"
#include "../../../SEH/seh.h"
#include "../../../Com/com.h"

extern localization_t localization;
extern char language_buffer[4096];

language_t Win_InitLocalization(void) {
    localization.language = NULL;
    localization.strings = NULL;
    FILE* f = FS_FileOpenReadText("localization.txt");
    if (f == NULL) {
        Com_Printf(0, "localization.txt not found.");
        return LANGUAGE_ENGLISH;
    }

    size_t fileSize = FS_FileGetSize(f);
    localization.language = language_buffer;
    uint readCount = FS_FileRead(language_buffer, fileSize, f);
    FS_FileClose(f);

    if (readCount == 0) {
        localization.language = NULL;
        return 0;
    }
    localization.language[readCount] = '\0';

    char* lang = localization.language;
    if (lang[0] == '\0')
        return 0;

    int i = 0;
    for (; lang[i] != '\n'; i++) {
        if (lang[i] == '\0') {
            return LANGUAGE_ENGLISH;
        }
    }

    lang[i] = '\0';
    localization.strings = &lang[i] + 1;

    int lang_index = LANGUAGE_ENGLISH;
    SEH_GetLanguageIndexForName(lang, &lang_index);
    return lang_index;
}

// TODO -----------------------------------------------------------------------
char* Win_LocalizeRef(const char* str) {
    return (char*)str;
    /*
    byte bVar1;
    char cVar2;
    ParseThreadInfo* pPVar3;
    char* pcVar4;
    byte* pbVar5;
    int iVar6;
    bool bVar7;

    Com_BeginParseSession("localization");
    pPVar3 = Com_GetParseThreadInfo();
    cVar2 = pPVar3->parseInfo[0].token[0];
    while (pbVar5 = (byte*)str, cVar2 != '\0') {
        do {
            bVar1 = pPVar3->parseInfo[0].token[0];
            bVar7 = bVar1 < *pbVar5;
            if (bVar1 != *pbVar5) {
            LAB_00527ba2:
                iVar6 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
                goto LAB_00527ba7;
            }
            if (bVar1 == 0) break;
            bVar1 = pPVar3->parseInfo[0].token[1];
            bVar7 = bVar1 < pbVar5[1];
            if (bVar1 != pbVar5[1]) goto LAB_00527ba2;
            pPVar3 = (ParseThreadInfo*)(pPVar3->parseInfo[0].token + 2);
            pbVar5 = pbVar5 + 2;
        } while (bVar1 != 0);
        iVar6 = 0;
    LAB_00527ba7:
        pPVar3 = Com_GetParseThreadInfo();
        if (pPVar3->parseInfo[0].token[0] == '\0') break;
        if (iVar6 == 0) {
            Com_EndParseSession();
            pcVar4 = va("%s", pPVar3);
            return pcVar4;
        }
        pPVar3 = Com_GetParseThreadInfo();
        cVar2 = pPVar3->parseInfo[0].token[0];
    }
    Com_EndParseSession();
    pcVar4 = va("%s", str);
    return pcVar4;
    */
}
// ----------------------------------------------------------------------------

void Win_ShutdownLocalization() {
    localization.language = NULL;
    localization.strings  = NULL;
}

#endif // _WIN32