#include "editor.h"
#include <gl/gl.h>

void DEBUG_Init_GL_Context(HDC DeviceContext)
{
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
    
    // get the best available match of pixel format for the device context   
    int iPixelFormat = ChoosePixelFormat(DeviceContext, &pfd); 
    
    // make that the pixel format of the device context  
    SetPixelFormat(DeviceContext, iPixelFormat, &pfd);
    
    HGLRC rc = wglCreateContext(DeviceContext);
    wglMakeCurrent(DeviceContext, rc);
}

void DEBUG_Render_GL(HDC hdc)
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    SwapBuffers(hdc);
}

int main(int ArgumentCount, char** Arguments)
{
    if(!Editor_Init()) return 1;
    Editor_Update();
    Editor_Shutdown();
    return 0;
}

editor* Editor_Init()
{
    core* Core = Core_Init();
    arena* EditorArena = Arena_Create(OS_Get_Allocator(), Mega(32));
    editor* Editor = Arena_Push_Struct(EditorArena, editor);
    
    Editor->Arena = EditorArena;
    Editor->RootPath = OS_Get_Application_Path(Editor->Arena);
    Editor->DataPath = Str8_Concat(Get_Base_Allocator(Editor->Arena), Editor->RootPath, Str8_Lit(Glue("Data", OS_FILE_DELIMTER)));
    Editor->Core  = Core;
    Editor->EditorUI = EditorUI_Init(EditorArena);
    
    os_window* MainWindow = OS_Create_Window(1920, 1080, Str8_Lit("AK Engine"), 0);
    HDC DEBUGDeviceContext = GetDC(((win32_window*)MainWindow)->Handle);
    DEBUG_Init_GL_Context(DEBUGDeviceContext);
    
    Editor->MainWindow = MainWindow;
    Editor_Set(Editor);
    return Editor;
}

void Editor_Update()
{
    editor* Editor = Editor_Get();
    HDC DEBUGDeviceContext = GetDC(((win32_window*)Editor->MainWindow)->Handle);
    
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
                    if(Event->Window == Editor->MainWindow)
                    {
                        IsLooping = false;
                    }
                    OS_Delete_Window(Event->Window);
                } break;
            }
            
            Event = OS_Get_Next_Event();
        }
        
        DEBUG_Render_GL(DEBUGDeviceContext);
    }
}

void Editor_Shutdown()
{
    //TODO(JJ): Handle case
}

global editor* G_Editor;
void Editor_Set(editor* Editor)
{
    G_Editor = Editor;
    if(Editor)
    {
        Core_Set(Editor->Core);
        EditorUI_Set(Editor->EditorUI);
    }
    else
    {
        Core_Set(NULL);
        EditorUI_Set(NULL);
    }
}

editor* Editor_Get()
{
    return G_Editor;
}

#include <Core/core.c>
#include <Editor_UI/editor_ui.c>