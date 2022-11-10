#include "editor.h"
#include <gl/gl.h>

int main(int ArgumentCount, char** Arguments)
{
    if(!Core_Init())
    {
        //TODO(JJ): Diagnostic and error logging
        return 1;
    }
    
    os_window* MainWindow = OS_Create_Window(1920, 1080, Str8_Lit("AK Engine"), 0);
    
    PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
        1,                     // version number  
        PFD_DRAW_TO_WINDOW |   // support window  
            PFD_SUPPORT_OPENGL |   // support OpenGL  
            PFD_DOUBLEBUFFER,      // double buffered  
        PFD_TYPE_RGBA,         // RGBA type  
        24,                    // 24-bit color depth  
        0, 0, 0, 0, 0, 0,      // color bits ignored  
        0,                     // no alpha buffer  
        0,                     // shift bit ignored  
        0,                     // no accumulation buffer  
        0, 0, 0, 0,            // accum bits ignored  
        32,                    // 32-bit z-buffer  
        0,                     // no stencil buffer  
        0,                     // no auxiliary buffer  
        PFD_MAIN_PLANE,        // main layer  
        0,                     // reserved  
        0, 0, 0                // layer masks ignored  
    }; 
    HDC  hdc; 
    int  iPixelFormat; 
    
    hdc = GetDC(((win32_window*)MainWindow)->Handle);
    
    // get the best available match of pixel format for the device context   
    iPixelFormat = ChoosePixelFormat(hdc, &pfd); 
    
    // make that the pixel format of the device context  
    SetPixelFormat(hdc, iPixelFormat, &pfd);
    
    HGLRC rc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, rc);
    
    bool32_t IsLooping = true;
    while(IsLooping)
    {
        OS_Poll_Events();
        
        const os_event* Event = OS_Get_Next_Event();
        while(Event)
        {
            switch(Event->Type)
            {
                case OS_EVENT_TYPE_WINDOW_CLOSED:
                {
                    if(Event->Window == MainWindow)
                    {
                        IsLooping = false;
                    }
                    OS_Delete_Window(Event->Window);
                } break;
            }
            
            Event = OS_Get_Next_Event();
        }
        
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        SwapBuffers(hdc);
    }
    
    Core_Shutdown();
    return 0;
}

#include <Core/core.c>