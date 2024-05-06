#include <core/core.h>
#include <profiler/profiler.h>

struct profiler {
    arena*                Arena;
    u32                   UpdateIndex;
    arena*                UpdateArenas[2];
    array<prof_stat>      Stats[2];
    hashmap<string, uptr> StatMap;
};
internal profiler* G_Profiler;

void Prof_Start_Capture() {
    Assert(!G_Profiler);
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    G_Profiler = Arena_Push_Struct(Arena, profiler);
    G_Profiler->Arena = Arena;
    Hashmap_Init(&G_Profiler->StatMap, G_Profiler->Arena);
    
    for(u32 i = 0; i < 2; i++) {
        G_Profiler->UpdateArenas[i] = Arena_Create(G_Profiler->Arena);
        Array_Init(&G_Profiler->Stats[i], G_Profiler->Arena);
    }
}

void Prof_End_Capture() {
    if(!G_Profiler) return;
}

void Prof_Begin(const char* Format, ...) {
    if(!G_Profiler) return;
}

void Prof_End() {
    if(!G_Profiler) return;
}

void Prof_Tick() {
    if(!G_Profiler) return;
    u32 Index = G_Profiler->UpdateIndex;
    G_Profiler->UpdateIndex = !G_Profiler->UpdateIndex;
    Array_Resize(&G_Profiler->Stats[G_Profiler->UpdateIndex], G_Profiler->Stats[Index].Count);
    for(uptr i = 0; i < G_Profiler->Stats[Index].Count; i++) {
        G_Profiler->Stats[G_Profiler->UpdateIndex][i] = {};
        G_Profiler->Stats[G_Profiler->UpdateIndex][i] = {
            .Name = G_Profiler->Stats[Index][i].Name
        };
    }
    Arena_Clear(G_Profiler->UpdateArenas[G_Profiler->UpdateIndex]);
}

span<prof_stat> Prof_Stats() {
    if(!G_Profiler) return {};
    return G_Profiler->Stats[!G_Profiler->UpdateIndex];
}