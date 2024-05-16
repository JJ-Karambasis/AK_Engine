#ifndef POSIX_H
#define POSIX_H

#include <core/core.h>
#include <math/math.h>
#include <os.h>
#include <os_event.h>
#include <sys/stat.h>

struct os_process {
    pid_t PID;
};

struct os {
    arena*                 Arena;
    async_pool<os_process> ProcessPool;
    os_event_manager       EventManager;
};

internal bool Posix_Create();

#endif