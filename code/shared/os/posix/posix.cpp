internal string Posix_Read_Symlink(allocator* Allocator, const char* Path) {

    uptr Size = 64;
    sptr ReturnCount = -1;

    for(;;) {
        scratch Scratch = Scratch_Get();
        char* Buffer = Scratch_Push_Array(&Scratch, Size, char);
        ReturnCount = readlink(Path, Buffer, Size);
        if(ReturnCount == -1) {
            return {};
        } else if(ReturnCount < Size) {
            string Result(Allocator, Buffer, ReturnCount);
            return Result;
        }

        Size *= 2;
    }
}

internal string Posix_Get_Executable_Path(allocator* Allocator) {
    string Result;
    if(String_Is_Null_Or_Empty(Result) && (access("/proc", F_OK) == 0)) {
        string FullPath = Posix_Read_Symlink(Allocator, "/proc/self/exe");
    }
    Assert(!String_Is_Null_Or_Empty(Result));
    return Result;
}

internal void Posix_Create(os* OS, const os_create_info& CreateInfo) {
    
    if(String_Is_Null_Or_Empty(OS->ExecutablePath)) {
        scratch Scratch = Scratch_Get();
        string ExecutableFilePath = Posix_Get_Executable_Path(&Scratch);
        OS->ExecutablePath = string(OS->Arena, String_Get_Path(ExecutableFilePath));
    }

    Async_Pool_Create(&OS->ProcessPool, OS->Arena, CreateInfo.MaxProcessCount);
}

string OS_Get_Executable_Path() {
    os* OS = OS_Get();
    Assert(OS);
    return OS->ExecutablePath;
}

bool OS_Directory_Exists(string Directory) {
    struct stat Stat;
    if(stat(Directory.Str, &Stat) == 0) {
        return S_ISDIR(Stat.st_mode);
    }
    return false;
}

bool OS_File_Exists(string File) {
    struct stat Stat;
    if(stat(File.Str, &Stat) == 0) {
        return S_ISREG(Stat.st_mode);
    }
    return false;
}

os_process_id OS_Exec_Process(string App, string Parameters) {
    os* OS = OS_Get();
    Assert(OS);
    
    pid_t PID = fork();
    if(PID == 0) {
        if(execlp(App.Str, App.Str, Parameters.Str, (char*)0) == -1) {
            printf("Dead\n");
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
internal os* OS_Get_Or_Set(os* OS) {
    if(OS) {
        G_OS = OS;
    }
    return G_OS;
}