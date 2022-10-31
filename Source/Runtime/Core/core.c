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

#include <Base/base.c>
#include <Memory/memory.c>

#ifdef EDITOR_BUILD
#include <Editor_OS/editor_os.c>
#else
#include <Runtime_OS/runtime_os.c>
#endif