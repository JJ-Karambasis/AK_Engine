#ifndef CORE_H
#define CORE_H

#include <Base/base.h>
#include <Containers/containers.h>
#include <Memory/memory.h>
#include <Strings/strings.h>

#ifdef EDITOR_BUILD
#include <Editor_OS/editor_os.h>
#else
#include <Runtime_OS/runtime_os.h>
#endif

#include <Random/random.h>

typedef struct core core;

#include "Public/async_spin_lock.h"
#include "Public/thread_manager.h"

typedef struct core
{
    runtime_os*    OS;
    thread_manager ThreadManager;
} core;

core*           Core_Init();
void            Core_Shutdown();
thread_context* Core_Create_Thread(core_thread_callback* Callback, void* UserData);
void            Core_Delete_Thread(thread_context* ThreadContext);
void            Core_Wait_For_Thread(thread_context* ThreadContext);
thread_context* Core_Get_Thread_Context();
void            Core_Wait_For_All_Threads();
void            Core_Set(core* Core);
core*           Core_Get();

#endif