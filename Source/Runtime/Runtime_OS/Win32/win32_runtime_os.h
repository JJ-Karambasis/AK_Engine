#ifndef WIN32_RUNTIME_OS_H
#define WIN32_RUNTIME_OS_H

#include <Core/core.h>
#include <windows.h>
#include "Public/win32_allocator.h"
#include "Public/win32_thread.h"
#include "Public/win32_file.h"

typedef struct win32_runtime_os
{
    runtime_os      OS;
    win32_allocator Allocator;
    arena*          Arena;
    win32_thread*   FreeThreads;
    win32_file*     FreeFiles;
    uint64_t        ClockFrequency;
} win32_runtime_os;

void Win32_Register_Window_Class(const wchar_t* ClassName, WNDPROC WindowProc);
HWND Win32_Create_Window(uint32_t Width, uint32_t Height, str8 WindowName, const wchar_t* ClassName, DWORD Style, DWORD ExtendedStyle);
void Win32_Destroy_Window(HWND Window);
v2i  Win32_Get_Window_Dim(HWND Window);

#endif