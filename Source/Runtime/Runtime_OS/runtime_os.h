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
allocator*  OS_Get_Allocator();
void        OS_Get_Random_Seed(void* Data, uint32_t Size);
void        OS_Set(runtime_os* OS);
runtime_os* OS_Get();

#ifdef OS_WIN32
#include "Win32/win32_runtime_os.h"
#endif

#endif