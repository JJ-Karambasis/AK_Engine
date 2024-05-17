internal bool Posix_Create() {
    os* OS = OS_Get();
    OS_Event_Manager_Create(&OS->EventManager, 1);
    Async_Pool_Create(&OS->ProcessPool, OS->Arena, OS_MAX_PROCESS_COUNT);
    return true;
}

os_process_id OS_Exec_Process(string App, string Parameters) {
    os* OS = OS_Get();
    Assert(OS);
    
    pid_t PID = fork();
    if(PID == 0) {
        if(execlp(App.Str, App.Str, Parameters.Str, (char*)0) == -1) {
            exit(-1);
        } else {
            exit(0);
        }
    }

    Assert(PID != 0);
    if(PID <= 0) {
        return 0;
    }

    async_handle<os_process> Handle = Async_Pool_Allocate(&OS->ProcessPool);
    os_process* Process = Async_Pool_Get(&OS->ProcessPool, Handle);
    Process->PID = PID;
    return Handle.ID;
}

internal os_process* OS_Get_Process(os_process_id ID) {
    os* OS = OS_Get();
    Assert(OS);
    async_handle<os_process> Handle(ID);
    os_process* Process = Async_Pool_Get(&OS->ProcessPool, Handle);
    Assert(Process);
    return Process;
}

void OS_Wait_For_Process(os_process_id ID) {
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return;
    int ReturnStatus;
    waitpid(Process->PID, &ReturnStatus, 0);
}

os_process_status OS_Process_Status(os_process_id ID) {
    os_process_status Result = OS_PROCESS_STATUS_NONE;
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return Result;

    int ReturnStatus;
    pid_t PID = waitpid(Process->PID, &ReturnStatus, WNOHANG);
    if(PID != -1) {
        Result = PID == 0 ? OS_PROCESS_STATUS_ACTIVE : OS_PROCESS_STATUS_EXIT;
    }
    return Result;
}

int OS_Process_Exit(os_process_id ID) {
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return -1;

    int Result = -1;

    int ReturnStatus;
    if(waitpid(Process->PID, &ReturnStatus, 0) == -1) {
        return -1;
    }

    if(WIFEXITED(ReturnStatus)) {
        Result = WEXITSTATUS(ReturnStatus);
    }
    os* OS = OS_Get();
    Async_Pool_Free(&OS->ProcessPool, async_handle<os_process>(ID));
    return Result;
}

global os* G_OS;
os* OS_Get() {
    Assert(G_OS);
    return G_OS;
}

void OS_Set(os* OS) {
    Assert(!G_OS);
    G_OS = OS;
}

#include <core/core.cpp>
#include <math/math.cpp>
#include <os_event.cpp>