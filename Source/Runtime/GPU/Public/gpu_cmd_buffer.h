#ifndef GPU_CMD_BUFFER_H
#define GPU_CMD_BUFFER_H

#define GPU_CMD_UPLOAD_TEXTURE_PROC(name) void name(gpu_cmd_buffer* _CmdBuffer, gpu_texture2D* _DstTexture, uint32_t DstOffsetX, uint32_t DstOffsetY, void* SrcTexels, uint32_t SrcWidth, uint32_t SrcHeight)
typedef GPU_CMD_UPLOAD_TEXTURE_PROC(gpu_cmd_upload_texture_proc);

typedef union gpu_color_clear_attachment
{
    int32_t  Integer[4];
    uint32_t Unsigned[4];
    float    Float[4];
} gpu_color_clear_attachment;

typedef struct gpu_depth_stencil_clear_attachment
{
    float    Depth;
    uint32_t Stencil;
} gpu_depth_stencil_clear_attachment;

typedef struct gpu_clear_attachments
{
    uint32_t                            ColorCount;
    gpu_color_clear_attachment*         ColorClears;
    gpu_depth_stencil_clear_attachment* DepthStencilClear;
} gpu_clear_attachments;

typedef enum gpu_ui_pass_cmd_type
{
    GPU_UI_PASS_CMD_TYPE_NONE,
    GPU_UI_PASS_CMD_TYPE_DRAW_RECTANGLE
} gpu_ui_pass_cmd_type;

typedef struct gpu_ui_pass_cmd
{
    gpu_ui_pass_cmd_type    Type;
    struct gpu_ui_pass_cmd* Next;
} gpu_ui_pass_cmd;

typedef struct gpu_ui_pass_draw_rectangle
{
    gpu_ui_pass_cmd Cmd;
    v2              Min;
    v2              Max;
    v4              Color;
} gpu_ui_pass_draw_rectangle;

typedef struct gpu_ui_pass
{
    gpu_clear_attachments ClearAttachments;
    gpu_framebuffer*      Framebuffer;
    
    arena*           CmdArena;
    gpu_ui_pass_cmd* FirstCmd;
    gpu_ui_pass_cmd* LastCmd;
} gpu_ui_pass;

typedef enum gpu_cmd_type
{
    GPU_CMD_TYPE_NONE,
    GPU_CMD_TYPE_BEGIN_UI_PASS,
    GPU_CMD_TYPE_UPLOAD_TEXTURE,
    GPU_CMD_TYPE_COPY_TEXTURE_TO_DISPLAY
} gpu_cmd_type;

typedef struct gpu_cmd
{
    gpu_cmd_type    Type;
    struct gpu_cmd* Next;
} gpu_cmd;

typedef struct gpu_cmd_begin_ui_pass
{
    gpu_cmd     Cmd;
    gpu_ui_pass UIPass;
} gpu_cmd_begin_ui_pass;

typedef struct gpu_cmd_copy_texture_to_display
{
    gpu_cmd        Cmd;
    gpu_display*   DstDisplay;
    uint32_t       DstOffsetX;
    uint32_t       DstOffsetY;
    gpu_texture2D* SrcTexture;
    uint32_t       SrcOffsetX;
    uint32_t       SrcOffsetY;
    uint32_t       Width;
    uint32_t       Height;
} gpu_cmd_copy_texture_to_display;

typedef struct gpu_cmd_buffer_vtable
{
    gpu_cmd_upload_texture_proc* Upload_Texture;
} gpu_cmd_buffer_vtable;

typedef struct gpu_cmd_buffer
{
    gpu_cmd_buffer_vtable* _VTable;
    
    arena*   CmdArena;
    gpu_cmd* FirstCmd;
    gpu_cmd* LastCmd;
} gpu_cmd_buffer;

typedef struct gpu_framebuffer_pass_info
{
    gpu_clear_attachments Clear;
    gpu_framebuffer* Framebuffer;
} gpu_framebuffer_pass_info;

typedef struct gpu_ui_pass_begin_info
{
    gpu_framebuffer_pass_info FramebufferInfo;
} gpu_ui_pass_begin_info;

void            GPU_Cmd_Buffer_Reset(gpu_cmd_buffer* CmdBuffer);
gpu_ui_pass*    GPU_Cmd_Buffer_Begin_UI_Pass(gpu_cmd_buffer* CmdBuffer, gpu_ui_pass_begin_info* BeginInfo);
#define         GPU_Cmd_Upload_Texture(CmdBuffer, DstTexture, DstOffsetX, DstOffsetY, SrcTexels, SrcWidth, SrcHeight) (CmdBuffer)->Upload_Tesxture(CmdBuffer, DstTexture, DstOffsetX, DstOffsetY, SrcTexels, SrcWidth, SrcHeight)
//void            GPU_Cmd_Upload_Async_Texture();
void            GPU_Cmd_Copy_Texture_To_Display(gpu_cmd_buffer* CmdBuffer, 
                                                gpu_display* Display, uint32_t DisplayOffsetX, uint32_t DisplayOffsetY, gpu_texture2D* Texture, uint32_t TextureOffsetX, uint32_t TextureOffsetY, 
                                                uint32_t Width, uint32_t Height);
void            GPU_UI_Pass_Draw_Rectangle(gpu_ui_pass* UIPass, v2 Min, v2 Max, v4 Color);

#endif