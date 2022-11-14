#ifndef RUNTIME_OS_H
#define RUNTIME_OS_H

typedef struct runtime_os
{
    void* EditorOS;
} runtime_os;

typedef struct allocator allocator;

#include "Public/os_thread.h"
#include "Public/os_file.h"

runtime_os* OS_Init();
void        OS_Shutdown();
void        OS_Get_Random_Seed(void* Data, uint32_t Size);
str8        OS_Get_Application_Path(arena* Arena);
uint64_t    OS_QPC();
double      OS_High_Res_Elapsed_Time(uint64_t End, uint64_t Start);
allocator*  OS_Get_Allocator();
void        OS_Set(runtime_os* OS);
runtime_os* OS_Get();
void        _OS_Debug_Log(str8 Format, ...);

#ifdef OS_WIN32
#include "Win32/win32_runtime_os.h"
#endif

#ifdef DEBUG_BUILD
#define OS_Debug_Log(Format, ...) _OS_Debug_Log(Format, __VA_ARGS__)
#else
#define OS_Debug_Log(Format, ...)
#endif

#endif