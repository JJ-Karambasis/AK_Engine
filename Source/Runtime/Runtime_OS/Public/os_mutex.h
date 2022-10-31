#ifndef OS_MUTEX_H
#define OS_MUTEX_H

typedef struct os_mutex os_mutex;

os_mutex* OS_Create_Mutex();
void      OS_Delete_Mutex(os_mutex* Mutex);
void      OS_Lock_Mutex(os_mutex* Mutex);
void      OS_Unlock_Mutex(os_mutex* Mutex);

#endif
