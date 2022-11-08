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