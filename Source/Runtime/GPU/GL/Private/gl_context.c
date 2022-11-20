#ifdef OS_WIN32
#include "gl_win32_context.c"
#else
#error Not Implemented
#endif

gl_context* GL_Context_Manager__Allocate_Context(gl_context_manager* ContextManager)
{
    gl_context* Context = ContextManager->FreeContexts;
    if(!Context) Context = GL_Context_Manager__Platform_Allocate_Context(ContextManager->Arena);
    else SLL_Pop_Front(ContextManager->FreeContexts);
    GL_Context_Manager__Platform_Clear_Context(Context);
    return Context;
}

void GL_Context_Manager__Free_Context(gl_context_manager* ContextManager, gl_context* Context)
{
    GL_Context_Manager__Platform_Delete_Context(Context);
    SLL_Push_Front(ContextManager->FreeContexts, Context);
}

void GL__Initial_Context_Data(gl_context* Context)
{
    glGenVertexArrays(1, &Context->EmptyVAO);
}

gl_context_manager* GL_Context_Manager_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Kilo(8));
    gl_context_manager* Result = Arena_Push_Struct(Arena, gl_context_manager);
    Zero_Struct(Result, gl_context_manager);
    Result->Arena = Arena;
    return Result;
}

void GL_Context_Manager_Delete(gl_context_manager* ContextManager)
{
    Arena_Delete(ContextManager->Arena);
}

gl_context* GL_Context_Manager_Create_Default(gl_context_manager* ContextManager)
{
    gl_context* Context = GL_Context_Manager__Allocate_Context(ContextManager);
    if(!GL_Context_Manager__Platform_Set_Default(Context))
    {
        GL_Context_Manager__Free_Context(ContextManager, Context);
        return NULL;
    }
    return Context;
}

bool8_t GL_Context_Manager_Set_Device_Context(gl_context_manager* ContextManager, gl_device* Device)
{
    if(!GL_Context_Manager__Platform_Set_Device_Context(Device))
        return false;
    
    GL_Context_Make_Current(Device->Context);
    
    glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)GL__Platform_Get_Proc("glGetFramebufferAttachmentParameteriv");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GL__Platform_Get_Proc("glBindFramebuffer");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GL__Platform_Get_Proc("glGenFramebuffers");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GL__Platform_Get_Proc("glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GL__Platform_Get_Proc("glCheckFramebufferStatus");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GL__Platform_Get_Proc("glDeleteFramebuffers");
    glCreateShader = (PFNGLCREATESHADERPROC)GL__Platform_Get_Proc("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)GL__Platform_Get_Proc("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)GL__Platform_Get_Proc("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)GL__Platform_Get_Proc("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GL__Platform_Get_Proc("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)GL__Platform_Get_Proc("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)GL__Platform_Get_Proc("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)GL__Platform_Get_Proc("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GL__Platform_Get_Proc("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GL__Platform_Get_Proc("glGetProgramInfoLog");
    glDetachShader = (PFNGLDETACHSHADERPROC)GL__Platform_Get_Proc("glDetachShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)GL__Platform_Get_Proc("glDeleteShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GL__Platform_Get_Proc("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)GL__Platform_Get_Proc("glUseProgram");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GL__Platform_Get_Proc("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GL__Platform_Get_Proc("glBindVertexArray");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GL__Platform_Get_Proc("glGetUniformLocation");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)GL__Platform_Get_Proc("glDrawBuffers");
    glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)GL__Platform_Get_Proc("glClearBufferfv");
    glUniform1i = (PFNGLUNIFORM1IPROC)GL__Platform_Get_Proc("glUniform1i");
    glUniform1ui = (PFNGLUNIFORM1UIPROC)GL__Platform_Get_Proc("glUniform1ui");
    glUniform2f = (PFNGLUNIFORM2FPROC)GL__Platform_Get_Proc("glUniform2f");
    glUniform4f = (PFNGLUNIFORM4FPROC)GL__Platform_Get_Proc("glUniform4f");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)GL__Platform_Get_Proc("glActiveTexture");
    glGenSamplers = (PFNGLGENSAMPLERSPROC)GL__Platform_Get_Proc("glGenSamplers");
    glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)GL__Platform_Get_Proc("glSamplerParameteri");
    glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)GL__Platform_Get_Proc("glDeleteSamplers");
    glBindSampler = (PFNGLBINDSAMPLERPROC)GL__Platform_Get_Proc("glBindSampler");
    glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)GL__Platform_Get_Proc("glBlendFuncSeparate");
    
#ifdef DEBUG_BUILD
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)GL__Platform_Get_Proc("glDebugMessageCallback");
    glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)GL__Platform_Get_Proc("glDebugMessageControl");
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GL_Debug_Message_Callback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
    
    GL__Initial_Context_Data(Device->Context);
    
    return true;
}

void GL_Context_Make_Current(gl_context* Context)
{
    GL_Context__Platform_Make_Current(Context);
}

void GL_Context_Present(gl_context* Context)
{
    GL_Context__Platform_Present(Context);
}

global gpu_display_manager_vtable G_DisplayManagerVTable =
{
    GL_Display_Manager_Create_Display,
    GL_Display_Manager_Delete_Display,
    GL_Display_Manager_Present_Displays
};

gl_display_manager* GL_Display_Manager_Create(allocator* Allocator, gl_context_manager* ContextManager, gl_resource_manager* ResourceManager)
{
    arena* Arena = Arena_Create(Allocator, Kilo(8));
    gl_display_manager* Result = Arena_Push_Struct(Arena, gl_display_manager);
    Zero_Struct(Result, gl_display_manager);
    Result->Arena = Arena;
    Result->ContextManager = ContextManager;
    Result->ResourceManager = ResourceManager;
    Set_VTable(&Result->DisplayManager, &G_DisplayManagerVTable);
    return Result;
}

void GL_Display_Manager_Delete(gl_display_manager* DisplayManager)
{
    Arena_Delete(DisplayManager->Arena);
}

global gpu_display_vtable G_DisplayVTable =
{
    GL_Display_Resize
};

GPU_CREATE_DISPLAY(GL_Display_Manager_Create_Display)
{
    gl_display_manager* DisplayManager = (gl_display_manager*)_DisplayManager;
    gl_device* CurrentDevice = DisplayManager->ContextManager->DeviceContext.Device;
    
    
    gl_context* Context = GL_Context_Manager__Allocate_Context(DisplayManager->ContextManager);
    bool32_t HasCreatedContext = false;
#ifdef OS_WIN32
    HasCreatedContext = GL_Context_Manager__Win32_Create_Context((gl_win32_context*)Context, Window, (gl_win32_context*)CurrentDevice->Context);
#else
#error Not Implemented
#endif
    if(!HasCreatedContext)
    {
        GL_Context_Manager__Free_Context(DisplayManager->ContextManager, Context);
        return NULL;
    }
    
    GL_Context_Make_Current(Context);
    GL__Initial_Context_Data(Context);
    GL_Context_Make_Current(NULL);
    
    if(CurrentDevice) GL_Context_Make_Current(CurrentDevice->Context);
    
    gl_display* Display = DisplayManager->FreeDisplays;
    if(!Display) Display = Arena_Push_Struct(DisplayManager->Arena, gl_display);
    else SLL_Pop_Front(DisplayManager->FreeDisplays);
    
    v2i Dim = GL_Context__Platform_Get_Dim(Context);
    
    Display->Context = Context;
    Display->Width = Safe_S64_U32(Dim.x);
    Display->Height = Safe_S64_U32(Dim.y);
    //Display->Texture = (gl_texture2D*)GL_Resource_Manager_Create_Texture2D((gpu_resource_manager*)DisplayManager->ResourceManager, Safe_S64_U32(Dim.x), Safe_S64_U32(Dim.y), GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_USAGE_RENDER_TARGET|GPU_TEXTURE_USAGE_SAMPLED);
    Set_VTable(&Display->Display, &G_DisplayVTable);
    return &Display->Display;
}

GPU_DELETE_DISPLAY(GL_Display_Manager_Delete_Display)
{
    gl_display_manager* DisplayManager = (gl_display_manager*)_DisplayManager;
    gl_display* Display = (gl_display*)_Display;
    GL_Context_Manager__Free_Context(DisplayManager->ContextManager, Display->Context);
    SLL_Push_Front(DisplayManager->FreeDisplays, Display);
}

GPU_PRESENT_DISPLAYS(GL_Display_Manager_Present_Displays)
{
    for(uint32_t DisplayIndex = 0; DisplayIndex < Count; DisplayIndex++)
    {
        const gl_display* Display = (const gl_display*)Displays[DisplayIndex];
        GL_Context_Present(Display->Context);
    }
}

GPU_DISPLAY_RESIZE(GL_Display_Resize)
{
    gl_display* Display =  (gl_display*)_Display;
    Display->Width = Width;
    Display->Height = Height;
}