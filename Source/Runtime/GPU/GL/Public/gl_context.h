#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

typedef struct gl_texture2D gl_texture2D;

typedef struct gl_context 
{
    GLuint             EmptyVAO;
    struct gl_context* Next;
} gl_context;

typedef struct gl_context_manager
{
    arena*            Arena;
    gl_device_context DeviceContext;
    gl_context*       FreeContexts;
} gl_context_manager;

typedef struct gl_display
{
    gpu_display        Display;
    gl_context*        Context;
    gl_texture2D*      Texture;
    struct gl_display* Next;
} gl_display;

typedef struct gl_display_manager
{
    gpu_display_manager  DisplayManager;
    arena*               Arena;
    gl_context_manager*  ContextManager;
    gl_resource_manager* ResourceManager;
    gl_display*          FreeDisplays;
} gl_display_manager;

gl_context_manager* GL_Context_Manager_Create(allocator* Allocator);
void                GL_Context_Manager_Delete(gl_context_manager* ContextManager);
gl_context*         GL_Context_Manager_Create_Default(gl_context_manager* ContextManager);

bool8_t             GL_Context_Manager_Set_Device_Context(gl_context_manager* ContextManager, struct gl_device* Device);

void                GL_Context_Make_Current(gl_context* Context);
void                GL_Context_Present(gl_context* Context);

gl_display_manager* GL_Display_Manager_Create(allocator* Allocator, gl_context_manager* ContextManager, gl_resource_manager* ResourceManager);
void                GL_Display_Manager_Delete(gl_display_manager* DisplayManager);
GPU_CREATE_DISPLAY(GL_Display_Manager_Create_Display);
GPU_DELETE_DISPLAY(GL_Display_Manager_Delete_Display);
GPU_PRESENT_DISPLAYS(GL_Display_Manager_Present_Displays);
GPU_DISPLAY_RESIZE(GL_Display_Resize);

static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLDETACHSHADERPROC glDetachShader;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

#endif