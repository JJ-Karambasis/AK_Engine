DWORD WINAPI Win32__Thread_Proc(LPVOID Parameter)
{
    win32_thread* Thread = (win32_thread*)Parameter;
    return Thread->Callback(Thread->UserData);
}

os_thread* OS_Create_Thread(os_thread_callback* Callback, void* UserData)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    
    win32_thread* Thread = OS->FreeThreads;
    if(!Thread) Thread = Arena_Push_Struct(OS->Arena, win32_thread);
    else OS->FreeThreads = OS->FreeThreads->Next;
    Zero_Struct(Thread, win32_thread);
    
    Thread->Callback = Callback;
    Thread->UserData = UserData;
    
    Thread->Handle = CreateThread(NULL, 0, Win32__Thread_Proc, Thread, 0, NULL);
    return (os_thread*)Thread;
}

void OS_Wait_Thread(os_thread* Thread)
{
    win32_thread* OSThread = (win32_thread*)Thread;
    WaitForSingleObject(OSThread->Handle, INFINITE);
}

void OS_Delete_Thread(os_thread* Thread)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return;
    OS_Wait_Thread(Thread);
    
    win32_thread* OSThread = (win32_thread*)Thread;
    CloseHandle(OSThread->Handle);
    OSThread->Next = OS->FreeThreads;
    OS->FreeThreads = OSThread;
}

uint32_t OS_Get_Current_Thread_ID()
{
    return GetCurrentThreadId();
}

uint32_t OS_Get_Thread_ID(os_thread* Thread)
{
    win32_thread* OSThread = (win32_thread*)Thread;
    return GetThreadId(OSThread->Handle);
}
