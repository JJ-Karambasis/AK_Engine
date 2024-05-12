#ifndef POSIX_H
#define POSIX_H

#include <core/core.h>
#include <os/os.h>

#include <sys/stat.h>

struct os_process {
    pid_t PID;
};

struct os {
    arena*                 Arena;
    async_pool<os_process> ProcessPool;
    string                 ExecutablePath;
};

internal void Posix_Create(os* OS, const os_create_info& CreateInfo);
internal os* OS_Get_Or_Set(os* OS);

#endif