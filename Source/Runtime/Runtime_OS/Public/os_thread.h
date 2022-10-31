#ifndef OS_THREAD_H
#define OS_THREAD_H

#define OS_THREAD_CALLBACK(name) int32_t name(void* UserData)
typedef OS_THREAD_CALLBACK(os_thread_callback);

typedef struct os_thread os_thread;

os_thread* OS_Create_Thread(os_thread_callback* Callback, void* UserData);
void       OS_Wait_Thread(os_thread* Thread);
void       OS_Delete_Thread(os_thread* Thread);
uint32_t   OS_Get_Current_Thread_ID();
uint32_t   OS_Get_Thread_ID(os_thread* Thread);

#endif