core* Core_Init()
{
    local core Result;
    Zero_Struct(&Result, core);
    
    if(!OS_Init())
    {
        //TODO(JJ): Diagnostic and error logging
    }
    Result.OS = OS_Get();
    
#ifdef EDITOR_BUILD
    Result.OS->EditorOS = Editor_OS_Init();
#endif
    
    Result.ThreadManager = Thread_Manager_Create();
    
    Core_Set(&Result);
    return &Result;
}

void Core_Shutdown()
{
    core* Core = Core_Get();
    if(Core)
    {
        Thread_Manager_Delete(&Core->ThreadManager);
#ifdef EDITOR_BUILD
        Editor_OS_Shutdown();
#endif
        OS_Shutdown();
    }
    Core_Set(NULL);
}

thread_context* Core_Create_Thread(core_thread_callback* Callback, void* UserData)
{
    core* Core = Core_Get();
    if(!Core) return NULL;
    return Thread_Manager_Create_Thread(&Core->ThreadManager, Callback, UserData);
}

void Core_Delete_Thread(thread_context* ThreadContext)
{
    core* Core = Core_Get();
    if(!Core) return;
    Thread_Manager_Delete_Thread(&Core->ThreadManager, ThreadContext);
}

void Core_Wait_For_Thread(thread_context* ThreadContext)
{
    OS_Wait_Thread(ThreadContext->Thread);
}

thread_context* Core_Get_Thread_Context()
{
    core* Core = Core_Get();
    if(!Core) return NULL;
    return Thread_Manager_Get_Thread_Context(&Core->ThreadManager);
}

void Core_Wait_For_All_Threads()
{
    core* Core = Core_Get();
    if(!Core) return;
    Thread_Manager_Wait_For_All_Threads(&Core->ThreadManager);
}

global core* G_Core;
void Core_Set(core* Core)
{
    G_Core = Core;
    if(Core) OS_Set(Core->OS);
    else OS_Set(NULL);
}

core* Core_Get()
{
    return G_Core;
}

#include "Private/thread_manager.c"
#include "Private/async_spin_lock.c"

#include <Base/base.c>
#include <Memory/memory.c>
#include <Strings/strings.c>
#include <Random/random.c>
#include <Math/math.c>

#ifdef EDITOR_BUILD
#include <Editor_OS/editor_os.c>
#else
#include <Runtime_OS/runtime_os.c>
#endif