#include <GL/wglext.h>
#define WIN32_GL_WINDOW_CLASS L"AK_Engine_GL_Win32_Window_Class_Temp_Context"

global PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
global PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
global PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

typedef struct gl_win32_context
{
    gl_context               Context;
    bool32_t                 HasAllocatedWindow;
    HWND                     Window;
    HDC                      DeviceContext;
    HGLRC                    RenderContext;
    struct gl_win32_context* Next;
} gl_win32_context;

void* GL__Platform_Get_Proc(const char* ProcName)
{
    return (void*)wglGetProcAddress(ProcName);
}

void GL_Context_Manager__Win32_Release_DC(gl_win32_context* Context)
{
    wglDeleteContext(Context->RenderContext);
    ReleaseDC(Context->Window, Context->DeviceContext);
}

void WGL__Set_Pixel_Format(HDC DeviceContext, int32_t TargetPixelFormatIndex)
{
    PIXELFORMATDESCRIPTOR PixelFormat;
    DescribePixelFormat(DeviceContext, TargetPixelFormatIndex, sizeof(PixelFormat), &PixelFormat);
    SetPixelFormat(DeviceContext, TargetPixelFormatIndex, &PixelFormat);
}

bool8_t GL_Context_Manager__Win32_Create_Legacy_Context(gl_win32_context* Context)
{
    Context->DeviceContext = GetDC(Context->Window);
    
    PIXELFORMATDESCRIPTOR TargetPixelFormat;
    Zero_Struct(&TargetPixelFormat, PIXELFORMATDESCRIPTOR);
    TargetPixelFormat.nSize      = sizeof(TargetPixelFormat);
    TargetPixelFormat.nVersion   = 1;
    TargetPixelFormat.iPixelType = PFD_TYPE_RGBA;
    TargetPixelFormat.dwFlags    = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    TargetPixelFormat.cColorBits = 32;
    TargetPixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    int32_t TargetPixelFormatIndex = ChoosePixelFormat(Context->DeviceContext, &TargetPixelFormat);
    WGL__Set_Pixel_Format(Context->DeviceContext, TargetPixelFormatIndex);
    
    Context->RenderContext = wglCreateContext(Context->DeviceContext);
    return true;
}

bool8_t GL_Context_Manager__Win32_Create_Modern_Context(gl_win32_context* Context, HGLRC SharedContext)
{
    Context->DeviceContext = GetDC(Context->Window);
    
    int AttribList[] = 
    {
        WGL_DRAW_TO_WINDOW_ARB, TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, TRUE, 
        WGL_DOUBLE_BUFFER_ARB, TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32, 
        0
    };
    
    int32_t TargetPixelFormatIndex;
    uint32_t NumFormats;
    wglChoosePixelFormatARB(Context->DeviceContext, AttribList, 0, 1, &TargetPixelFormatIndex, &NumFormats);
    WGL__Set_Pixel_Format(Context->DeviceContext, TargetPixelFormatIndex);
    
    int ContextFlags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#ifdef DEBUG_BUILD
    ContextFlags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif
    
    int Attributes[] = 
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, ENGINE_GL_MAJOR_VERSION,
        WGL_CONTEXT_MINOR_VERSION_ARB, ENGINE_GL_MINOR_VERSION,
        WGL_CONTEXT_FLAGS_ARB, ContextFlags, 
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    
    Context->RenderContext = wglCreateContextAttribsARB(Context->DeviceContext, SharedContext, Attributes);
    if(!Context->RenderContext) return false;
    
    return true;
}

gl_context* GL_Context_Manager__Platform_Allocate_Context(arena* Arena)
{
    gl_win32_context* Context = Arena_Push_Struct(Arena, gl_win32_context);
    return &Context->Context;
}

void GL_Context_Manager__Platform_Delete_Context(gl_context* _Context)
{
    gl_win32_context* Context = (gl_win32_context*)_Context;
    GL_Context_Manager__Win32_Release_DC(Context);
    if(Context->HasAllocatedWindow)
        Win32_Destroy_Window(Context->Window);
}

void GL_Context_Manager__Platform_Clear_Context(gl_context* Context)
{
    Zero_Struct(Context, gl_win32_context);
}

bool8_t GL_Context_Manager__Platform_Set_Default(gl_context* _Context)
{
    WNDCLASSEXW TempWindowClass;
    if(!GetClassInfoExW(GetModuleHandle(0), WIN32_GL_WINDOW_CLASS, &TempWindowClass))
        Win32_Register_Window_Class(WIN32_GL_WINDOW_CLASS, DefWindowProcW);
    
    gl_win32_context* Context = (gl_win32_context*)_Context;
    Context->HasAllocatedWindow = true;
    Context->Window = Win32_Create_Window(0, 0, Str8_Lit("AK_Engine_Temp_GL_Window"), WIN32_GL_WINDOW_CLASS, 0, 0);
    if(!GL_Context_Manager__Win32_Create_Legacy_Context(Context)) return false;
    
    return true;
}
bool8_t GL_Context_Manager__Platform_Set_Device_Context(gl_device* Device)
{
    gl_win32_context* Context = (gl_win32_context*)Device->Context;
    
    GL_Context_Make_Current(Device->Context);
    
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    
    if(!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
        return false;
    
    GL_Context_Make_Current(NULL);
    
    GL_Context_Manager__Win32_Release_DC(Context);
    if(!GL_Context_Manager__Win32_Create_Modern_Context(Context, NULL)) return false;
    
    return true;
}

bool8_t GL_Context_Manager__Win32_Create_Context(gl_win32_context* Context, HWND Window, gl_win32_context* DeviceContext)
{
    Context->HasAllocatedWindow = false;
    Context->Window = Window;
    if(!GL_Context_Manager__Win32_Create_Modern_Context(Context, DeviceContext->RenderContext)) return false;
    //if(!wglShareLists(Context->RenderContext, DeviceContext->RenderContext)) return false;
    return true;
}

void GL_Context__Platform_Make_Current(gl_context* _Context)
{
    gl_win32_context* Context = (gl_win32_context*)_Context;
    if(Context)
        wglMakeCurrent(Context->DeviceContext, Context->RenderContext);
    else
        wglMakeCurrent(NULL, NULL);
}

void GL_Context__Platform_Present(gl_context* _Context)
{
    gl_win32_context* Context = (gl_win32_context*)_Context;
    SwapBuffers(Context->DeviceContext);
}

v2i GL_Context__Platform_Get_Dim(gl_context* _Context)
{
    gl_win32_context* Context = (gl_win32_context*)_Context;
    v2i Dim = Win32_Get_Window_Dim(Context->Window);
    return Dim;
}