#ifndef SHEENBIDI_UBA_H
#define SHEENBIDI_UBA_H
#include <SheenBidi.h>

struct uba {
    arena*                 Arena;
    array<uba_run>         Runs;
    array<uba_script_info> Scripts;
};

#endif
