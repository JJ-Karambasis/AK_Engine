#ifndef UBA_H
#define UBA_H

enum uba_run_type {
    UBA_RUN_TYPE_INVALID,
    UBA_RUN_TYPE_LTR,
    UBA_RUN_TYPE_RTL,
    UBA_RUN_TYPE_COUNT
};

struct uba_run {
    uptr         Offset;
    uptr         Length;
    uba_run_type Type;
};

enum uba_script_type {
    UBA_SCRIPT_TYPE_NONE,
    UBA_SCRIPT_TYPE_COMMON,
    UBA_SCRIPT_TYPE_COUNT
};

struct uba_script {
    uptr            Offset;
    uptr            Length;
    uba_script_type Type;
};

struct uba;
uba*             UBA_Allocate(allocator* Allocator, string Text);
span<uba_run>    UBA_Get_Runs(uba* UBA);
span<uba_script> UBA_Get_Scripts(uba* UBA);
void             UBA_Free(uba* UBA);

#include "sheenbidi/sheenbidi_uba.h"

#endif