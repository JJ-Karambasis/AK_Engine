#ifndef GL_WIN32_DEVICE_H
#define GL_WIN32_DEVICE_H

#include <wglext.h>

typedef struct gl_win32_context
{
    bool32_t                 HasAllocatedWindow;
    HWND                     Window;
    HDC                      DeviceContext;
    HGLRC                    RenderContext;
    struct gl_win32_context* Next;
} gl_win32_context;


#endif