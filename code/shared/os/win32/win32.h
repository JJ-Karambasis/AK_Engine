#ifndef WIN32_H
#define WIN32_H

#include <core/core.h>
#include <os/os.h>

struct os_process {
    PROCESS_INFORMATION Information;
};

struct os {
    arena*                 Arena;
    string                 ExecutablePath;
    async_pool<os_process> ProcessPool;
};

internal os* OS_Get_Or_Set(os* OS);

#endif