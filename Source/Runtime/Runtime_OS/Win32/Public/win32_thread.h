#ifndef WIN32_THREAD_H
#define WIN32_THREAD_H

typedef struct win32_thread
{
    HANDLE               Handle;
    os_thread_callback*  Callback;
    void*                UserData;
    struct win32_thread* Next;
} win32_thread;

#endif