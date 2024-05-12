#include "osx.h"

internal string OSX_Get_Executable_Path(allocator* Allocator) {
    local_persist char TempBuffer[1];
    u32 BufferSize = 1;
    _NSGetExecutablePath(TempBuffer, &BufferSize);

    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, BufferSize);
    _NSGetExecutablePath(Buffer, &BufferSize);

    return string(Buffer, BufferSize-1);
}

os* OS_Create(const os_create_info& CreateInfo) {
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    osx_os* OS = Arena_Push_Struct(Arena, osx_os);
    OS->Arena = Arena;
    OS_Set(OS);

    scratch Scratch = Scratch_Get();
    string ExecutableFilePath = OSX_Get_Executable_Path(&Scratch);
    OS->ExecutablePath = string(OS->Arena, String_Get_Path(ExecutableFilePath));

    Posix_Create(OS, CreateInfo);
    return OS;
}

void OS_Delete();

os* OS_Get() {
    return (os*)OS_Get_Or_Set(nullptr);
}

void OS_Set(os* OS) {
    OS_Get_Or_Set(OS);
}

internal osx_os* OSX_Get_Or_Set(osx_os* OS) {
    return (osx_os*)OS_Get_Or_Set(OS);
}
#include <os/posix/posix.cpp>