GPU_CREATE_DISPLAY(GL_Create_Display)
{
    gl_display_manager* DisplayManager = (gl_display_manager*)_DisplayManager;
    
    gl_device* CurrentDevice = &DisplayManager->ContextManager->DeviceContext->Device;
    
    gl_context* Context = NULL;
    v2i Dim;
#ifdef OS_WIN32
    Dim = Win32_Get_Window_Dim(Window);
    Context = GL_Init_Win32_Window_Context(DisplayManager->GL, Window, CurrentDevice->Context);
#else
#error Not Implemented
#endif
    
    if(!Context) return NULL;
    
    if(CurrentDevice) GL_Context_Make_Current(CurrentDevice->Context);
    
    gl_display* Display = Display->FreeDisplays;
    if(!Display) Display = Arena_Push_Struct(DisplayManager->Arena, gl_display);
    else SLL_Pop_Front(Display->FreeDisplays);
    Set_VTable(&Display->Display, &G_GLDisplayVTable);
    Display->Context = Context;
    Display->Width = Safe_S64_U32(Dim.x);
    Display->Height = Safe_S64_U32(Dim.y);
    
    
    return &Display->Display;
}

GPU_DELETE_DISPLAY(GL_Delete_Display)
{
    gl_display_manager* DisplayManager = (gl_display_manager*)_DisplayManager;
    gl_display* Display = (gl_display*)_Display;
    
#ifdef OS_WIN32
    GL_Delete_Win32_Window_Context(DisplayManager->GL, Display->Context);
#else
#error Not Implemented
#endif
    
    SLL_Push_Front(DisplayManager->FreeDisplays, Display);
}