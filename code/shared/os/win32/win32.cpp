#include "win32.h"

internal string Win32_Get_Exe_File_Path(allocator* Allocator) {
    DWORD MemorySize = 1024;
    
    for(int Iterations = 0; Iterations < 32; Iterations++) {
        scratch Scratch = Scratch_Get();
        wchar_t* Buffer = (wchar_t*)Scratch_Push(&Scratch, sizeof(wchar_t)*MemorySize);
        DWORD Size = GetModuleFileNameW(nullptr, Buffer, MemorySize);
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return string(Allocator, wstring(Buffer, Size));
        }
        MemorySize *= 2;
    }

    return string();
}

internal inline bool Win32_File_Exists(const wchar_t* Path) {
    DWORD Attrib = GetFileAttributesW(Path);
    return (Attrib != INVALID_FILE_ATTRIBUTES && !(Attrib & FILE_ATTRIBUTE_DIRECTORY));
}

internal inline bool Win32_Directory_Exists(const wchar_t* Path) {
    DWORD Attrib = GetFileAttributesW(Path);
    return (Attrib != INVALID_FILE_ATTRIBUTES && (Attrib & FILE_ATTRIBUTE_DIRECTORY));
}

os* OS_Create(const os_create_info& CreateInfo) {
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    os* OS = Arena_Push_Struct(Arena, os);
    OS->Arena = Arena;
    OS_Set(OS);

    scratch Scratch = Scratch_Get();
    string ExecutableFilePath = Win32_Get_Exe_File_Path(&Scratch);
    OS->ExecutablePath = string(OS->Arena, String_Get_Path(ExecutableFilePath));

    Async_Pool_Create(&OS->ProcessPool, OS->Arena, CreateInfo.MaxProcessCount);

    return OS;
}

void OS_Delete() {

}

os* OS_Get() {
    return OS_Get_Or_Set(nullptr);
}

void OS_Set(os* OS) {
    OS_Get_Or_Set(OS);
}

string OS_Get_Executable_Path() {
    os* OS = OS_Get();
    Assert(OS);
    return OS->ExecutablePath;    
}

bool OS_Directory_Exists(string Directory) {
    scratch Scratch = Scratch_Get();
    wstring DirectoryW(&Scratch, Directory);
    return Win32_Directory_Exists(DirectoryW.Str);
}

bool OS_File_Exists(string File) {
    scratch Scratch = Scratch_Get();
    wstring FileW(&Scratch, File);
    return Win32_File_Exists(FileW.Str);
}

os_process_id OS_Exec_Process(string App, string Parameters) {
    os* OS = OS_Get();
    Assert(OS);

    STARTUPINFOW StartupInfo = {
        .cb = sizeof(StartupInfo)
    };

    PROCESS_INFORMATION ProcessInformation = {};

    scratch Scratch = Scratch_Get();

    string CmdLine = String_Concat(&Scratch, {App, String_Lit(" "), Parameters});
    wstring CmdLineW(&Scratch, CmdLine);

    if(!CreateProcessW(nullptr, (LPWSTR)CmdLineW.Str, nullptr, nullptr, FALSE, 0, nullptr, 
                       nullptr, &StartupInfo, &ProcessInformation)) {
        return 0;
    }
    
    async_handle<os_process> Handle = Async_Pool_Allocate(&OS->ProcessPool);
    os_process* Process = Async_Pool_Get(&OS->ProcessPool, Handle);
    Process->Information = ProcessInformation;

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
    WaitForSingleObject(Process->Information.hProcess, INFINITE);
}

os_process_status OS_Process_Status(os_process_id ID) {
    os_process_status Result = OS_PROCESS_STATUS_NONE;
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return Result;

    DWORD ExitCode;
    if(GetExitCodeProcess(Process->Information.hProcess, &ExitCode)) {
        Result = ExitCode == STILL_ACTIVE ? OS_PROCESS_STATUS_ACTIVE : OS_PROCESS_STATUS_EXIT; 
    }
    return Result;
}

int OS_Process_Exit(os_process_id ID) {
    DWORD ExitCode = (DWORD)-1;
    os_process* Process = OS_Get_Process(ID);
    if(GetExitCodeProcess(Process->Information.hProcess, &ExitCode)) {
        if(ExitCode == STILL_ACTIVE) {
            WaitForSingleObject(Process->Information.hProcess, INFINITE);
        }
        
        GetExitCodeProcess(Process->Information.hProcess, &ExitCode);
        Assert(ExitCode != STILL_ACTIVE);
    }
    os* OS = OS_Get();
    Async_Pool_Free(&OS->ProcessPool, async_handle<os_process>(ID));
    return (int)ExitCode;
}

global os* G_OS;
internal os* OS_Get_Or_Set(os* OS) {
    if(OS) {
        G_OS = OS;
    }
    return G_OS;
}