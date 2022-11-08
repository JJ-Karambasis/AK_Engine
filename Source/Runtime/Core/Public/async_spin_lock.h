#ifndef ASYNC_SPIN_LOCK_H
#define ASYNC_SPIN_LOCK_H

typedef struct async_spin_lock
{
    volatile int64_t Target;
    volatile int64_t Current;
} async_spin_lock;

void       Async_Lock(async_spin_lock* Lock);
void       Async_Unlock(async_spin_lock* Lock);

#endif