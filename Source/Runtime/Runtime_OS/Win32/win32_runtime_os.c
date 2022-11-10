runtime_os* OS_Init()
{
    local win32_runtime_os Result;
    Zero_Struct(&Result, win32_runtime_os);
    
    Win32_Get_Main_Allocator(&Result.Allocator);
    Result.Arena = Arena_Create(Get_Base_Allocator(&Result.Allocator), Kilo(32));
    
    OS_Set(&Result.OS);
    return &Result.OS;
}

void OS_Shutdown()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(OS)
    {
    }
    OS_Set(NULL);
}

void OS_Get_Random_Seed(void* Data, uint32_t Size)
{
    HCRYPTPROV Prov = 0;
    CryptAcquireContextW(&Prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(Prov, Size, (BYTE*)Data);
    CryptReleaseContext(Prov, 0);
}

str8 OS_Get_Application_Path(arena* Arena)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    DWORD     Capacity = 2048;
    uint64_t  Length   = 0;
    uint16_t* Buffer = NULL;
    for(;;)
    {
        Buffer = Arena_Push_Array(Scratch, uint16_t, Capacity);
        DWORD ReadSize = GetModuleFileNameW(0, (wchar_t*)Buffer, Capacity);
        if(ReadSize == Capacity && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            Capacity *= 4;
        }
        else
        {
            Length = ReadSize;
            break;
        }
    }
    
    str8 FullExePath = UTF16_To_UTF8(Get_Base_Allocator(Scratch), Str16(Buffer, Length));
    str8 FullPath    = Str8_Prefix(FullExePath, Str8_Find_Last(FullExePath, '\\'));
    str8 Result      = Str8_Concat(Get_Base_Allocator(Arena), Str8_Copy(Get_Base_Allocator(Scratch), FullPath), Str8_Lit("\\"));
    return Result;
}

allocator* OS_Get_Allocator()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    return Get_Base_Allocator(&OS->Allocator);
}

global runtime_os* G_OS;
void OS_Set(runtime_os* OS)
{
    G_OS = OS;
}

runtime_os* OS_Get()
{
    return G_OS;
}

#include "Private/win32_allocator.c"
#include "Private/win32_thread.c"
#include "Private/win32_file.c"