void Async_Lock(async_spin_lock* Lock)
{
    int64_t PreviousTarget = Atomic_Add64(&Lock->Target, 1);
    while(PreviousTarget != Lock->Current) { _mm_pause(); }
}

void Async_Unlock(async_spin_lock* Lock)
{
    Atomic_Increment64(&Lock->Current);
}