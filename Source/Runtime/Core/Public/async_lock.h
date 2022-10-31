#ifndef ASYNC_LOCK_H
#define ASYNC_LOCK_H

typedef struct async_lock
{
    os_mutex*        Mutex;
    int64_t          MaxSpinCount;
    volatile int64_t Target;
    volatile int64_t Current;
} async_lock;

async_lock Async_Lock_Create(int64_t MaxSpinCount);
void       Async_Lock_Delete(async_lock* Lock);
void       Async_Lock(async_lock* Lock);
void       Async_Unlock(async_lock* Lock);

#endif
