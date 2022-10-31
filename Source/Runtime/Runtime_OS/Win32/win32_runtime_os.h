#ifndef WIN32_RUNTIME_OS_H
#define WIN32_RUNTIME_OS_H

#include <Core/core.h>
#include <windows.h>
#include "Public/win32_allocator.h"

typedef struct win32_runtime_os
{
    runtime_os      OS;
    win32_allocator Allocator;
} win32_runtime_os;

#endif