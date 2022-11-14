#define WIN32_WINDOW_CLASS L"AK_Engine_Win32_Window_Class"
#define WIN32_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WIN32_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

runtime_os* OS_Init()
{
    local win32_runtime_os Result;
    Zero_Struct(&Result, win32_runtime_os);
    
    Win32_Get_Main_Allocator(&Result.Allocator);
    Result.Arena = Arena_Create(Get_Base_Allocator(&Result.Allocator), Kilo(32));
    QueryPerformanceFrequency((LARGE_INTEGER*)&Result.ClockFrequency);
    
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

uint64_t OS_QPC()
{
    uint64_t Result;
    QueryPerformanceCounter((LARGE_INTEGER*)&Result);
    return Result;
}

double OS_High_Res_Elapsed_Time(uint64_t End, uint64_t Start)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    return (double)(End-Start)/(double)OS->ClockFrequency;
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

void _OS_Debug_Log(str8 Format, ...)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    va_list List;
    va_start(List, Format.Str);
    str8 Str = Str8_FormatV(Get_Base_Allocator(Scratch), Format, List);
    va_end(List);
    
    str16 StrW = UTF8_To_UTF16(Get_Base_Allocator(Scratch), Str);
    OutputDebugStringW(StrW.Str);
}

void Win32_Register_Window_Class(const wchar_t* ClassName, WNDPROC WindowProc)
{
    WNDCLASSEXW WindowClass;
    Zero_Struct(&WindowClass, WNDCLASSEX);
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_OWNDC;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = ClassName;
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassExW(&WindowClass);
}

HWND Win32_Create_Window(uint32_t Width, uint32_t Height, str8 WindowName, const wchar_t* ClassName, DWORD Style, DWORD ExtendedStyle)
{
    RECT ClientRect;
    Zero_Struct(&ClientRect, RECT);
    ClientRect.right  = Width;
    ClientRect.bottom = Height;
    AdjustWindowRectEx(&ClientRect, Style, FALSE, ExtendedStyle);
    
    str16 WindowNameW = UTF8_To_UTF16(Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), WindowName);
    HWND Handle = CreateWindowExW(ExtendedStyle, ClassName, WindowNameW.Str, Style, 200, 200, ClientRect.right-ClientRect.left, ClientRect.bottom-ClientRect.top, NULL, NULL, GetModuleHandle(0), NULL);
    if(!Handle) return NULL;
    return Handle;
}

void Win32_Destroy_Window(HWND Window)
{
    DestroyWindow(Window);
}

v2i Win32_Get_Window_Dim(HWND Window)
{
    RECT Rect;
    GetClientRect(Window, &Rect);
    return V2i(Rect.right-Rect.left, Rect.bottom-Rect.top);
}

#include "Private/win32_allocator.c"
#include "Private/win32_thread.c"
#include "Private/win32_file.c"