#ifndef GL_DEVICE_H
#define GL_DEVICE_H

typedef struct gl_sampler gl_sampler;
typedef struct gl_framebuffer gl_framebuffer;
typedef struct gl_display gl_display; 
typedef struct gl_context gl_context;
typedef struct gl_texture2D gl_texture2D;
typedef struct gl_display_manager gl_display_manager;
typedef struct gl_resource_manager gl_resource_manager;
typedef struct gl_cmd_pool gl_cmd_pool;

typedef struct gl_device
{
    gpu_device  Device;
    gl_context* Context;
} gl_device;

typedef struct gl_device_ui_shader
{
    GLuint Program;
    GLint  MinUniformID;
    GLint  MaxUniformID;
    GLint  InvHalfResolutionUniformID;
    GLint  ColorUniformID;
    GLint  TextureUniformID;
} gl_device_ui_shader;

#define UI_SHADER_COLOR_INDEX 0
#define UI_SHADER_TEXTURE_INDEX 1
typedef struct gl_device_ui_shaders
{
    gl_device_ui_shader Shaders[2];
} gl_device_ui_shaders;

typedef struct gl_ui_rectangle_draw
{
    v2                           Min;
    v2                           Max;
    v4                           Color;
    struct gl_ui_rectangle_draw* Next;
} gl_ui_rectangle_draw;

typedef struct gl_ui_rectangle_draw_list
{
    gl_ui_rectangle_draw* First;
    gl_ui_rectangle_draw* Last;
    uint64_t              Count;
} gl_ui_rectangle_draw_list;

typedef struct gl_ui_render_pass 
{
    gpu_clear_attachments     ClearAttachments;
    gl_framebuffer*           Framebuffer;
    gl_ui_rectangle_draw_list ColorDraws;
    struct gl_ui_render_pass* Next;
} gl_ui_render_pass;

typedef struct gl_ui_render_pass_list
{
    gl_ui_render_pass* First;
    gl_ui_render_pass* Last;
    uint64_t           Count;
} gl_ui_render_pass_list;

typedef struct gl_copy_texture_to_display
{
    gl_display*   DstDisplay;
    uint32_t      DstOffsetX;
    uint32_t      DstOffsetY;
    gl_texture2D* SrcTexture;
    uint32_t      SrcOffsetX;
    uint32_t      SrcOffsetY;
    uint32_t      Width;
    uint32_t      Height;
    struct gl_copy_texture_to_display* Next;
} gl_copy_texture_to_display;

typedef struct gl_copy_texture_to_display_list
{
    gl_copy_texture_to_display* First;
    gl_copy_texture_to_display* Last;
    uint64_t Count;
} gl_copy_texture_to_display_list;

typedef struct gl_device_context
{
    gpu_device_context   DeviceContext;
    struct gl_device*    Device;
    arena*               Arena;
    gl_display_manager*  DisplayManager;
    gl_resource_manager* ResourceManager;
    gl_cmd_pool*         CmdPool;
    gl_device_ui_shaders UIShaders;
    gl_sampler*          DefaultLinearSampler;
} gl_device_context;

GPU_GET_DISPLAY_MANAGER(GL_Device_Context_Get_Display_Manager);
GPU_GET_RESOURCE_MANAGER(GL_Device_Context_Get_Resource_Manager);
GPU_ALLOCATE_CMD_BUFFER(GL_Device_Context_Allocate_Cmd_Buffer);
GPU_FREE_CMD_BUFFER(GL_Device_Context_Free_Cmd_Buffer);
GPU_DISPATCH_CMDS(GL_Device_Context_Dispatch_Cmds);

#endif