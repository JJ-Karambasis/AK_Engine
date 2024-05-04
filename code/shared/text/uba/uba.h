#ifndef UBA_H
#define UBA_H

enum uba_direction {
    UBA_DIRECTION_INVALID,
    UBA_DIRECTION_LTR,
    UBA_DIRECTION_RTL,
    UBA_DIRECTION_COUNT
};

struct uba_run {
    uptr          Offset;
    uptr          Length;
    uba_direction Direction;
};

enum uba_script {
    UBA_SCRIPT_NONE,
    UBA_SCRIPT_LATIN,
    UBA_SCRIPT_ARABIC,
    UBA_SCRIPT_DEVANAGARI,
    UBA_SCRIPT_COUNT
};

struct uba_script_info {
    uptr       Offset;
    uptr       Length;
    uba_script Script;
};

struct uba;
uba*                  UBA_Allocate(allocator* Allocator, string Text);
span<uba_run>         UBA_Get_Runs(uba* UBA);
span<uba_script_info> UBA_Get_Scripts(uba* UBA);
void                  UBA_Free(uba* UBA);

#include "sheenbidi/sheenbidi_uba.h"

#endif