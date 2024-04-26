#ifndef GDI_H
#define GDI_H

enum gdi_format {
    GDI_FORMAT_NONE,
    GDI_FORMAT_R8_UNORM,
    GDI_FORMAT_R8_SRGB,
    GDI_FORMAT_R8G8_UNORM,
    GDI_FORMAT_R8G8_SRGB,
    GDI_FORMAT_R8G8B8_UNORM,
    GDI_FORMAT_R8G8B8_SRGB,
    GDI_FORMAT_R8G8B8A8_UNORM,
    GDI_FORMAT_R8G8B8A8_SRGB,
    GDI_FORMAT_B8G8R8A8_UNORM,
    GDI_FORMAT_B8G8R8A8_SRGB,
    GDI_FORMAT_R16G16B16A16_FLOAT,
    GDI_FORMAT_R16_UINT,
    GDI_FORMAT_R32_UINT,
    GDI_FORMAT_R32_FLOAT,
    GDI_FORMAT_R32G32_FLOAT,
    GDI_FORMAT_R32G32B32_FLOAT,
    GDI_FORMAT_D16_UNORM,
    GDI_FORMAT_COUNT
};

bool       GDI_Is_Depth_Format(gdi_format Format);
uptr       GDI_Get_Bytes_Per_Pixel(gdi_format Format);
gdi_format GDI_Get_SRGB_Format(gdi_format Format);

template <typename type>
struct gdi_handle {
    u64 ID = 0;
    gdi_handle() = default;
    inline gdi_handle(u64 _ID) : ID(_ID) { }
    inline bool Is_Null() { return ID == 0; }
};

struct gdi_pipeline;
struct gdi_bind_group;
struct gdi_bind_group_layout;
struct gdi_framebuffer;
struct gdi_render_pass;
struct gdi_sampler;
struct gdi_texture_view;
struct gdi_texture;
struct gdi_buffer;
struct gdi_swapchain;
struct gdi_cmd_list;

struct gdi;
struct gdi_context;

struct gdi_shader {
    const_buffer ByteCode;
    string       EntryName;
};

struct gdi_vtx_attribute {
    string     Semantic;
    u32        SemanticIndex;
    uptr       ByteOffset;
    gdi_format Format;
};

struct gdi_vtx_buffer_binding {
    uptr                    ByteStride;
    span<gdi_vtx_attribute> Attributes;
};

enum gdi_comparison_func {
    GDI_COMPARISON_FUNC_LESS,
    GDI_COMPARISON_FUNC_COUNT
};

struct gdi_depth_state {
    bool DepthTestEnabled;
    bool DepthWriteEnabled;
    gdi_comparison_func ComparisonFunc = GDI_COMPARISON_FUNC_LESS;
};

enum gdi_topology {
    GDI_TOPOLOGY_TRIANGLE_LIST,
    GDI_TOPOLOGY_TRIANGLE_STRIP,
    GDI_TOPOLOGY_COUNT
};

enum gdi_blend {
    GDI_BLEND_ZERO,
    GDI_BLEND_ONE,
    GDI_BLEND_SRC_COLOR,
    GDI_BLEND_ONE_MINUS_SRC_COLOR,
    GDI_BLEND_DST_COLOR,
    GDI_BLEND_ONE_MINUS_DST_COLOR,
    GDI_BLEND_SRC_ALPHA,
    GDI_BLEND_ONE_MINUS_SRC_ALPHA,
    GDI_BLEND_DST_ALPHA,
    GDI_BLEND_ONE_MINUS_DST_ALPHA,
    GDI_BLEND_COUNT
};

enum gdi_blend_op {
    GDI_BLEND_OP_ADD,
    GDI_BLEND_OP_SUBTRACT,
    GDI_BLEND_OP_REVERSE_SUBTRACT,
    GDI_BLEND_OP_MIN,
    GDI_BLEND_OP_MAX,
    GDI_BLEND_OP_COUNT
};

struct gdi_blend_state {
    bool         BlendEnabled = false;
    gdi_blend    SrcColor     = GDI_BLEND_ONE;
    gdi_blend    DstColor     = GDI_BLEND_ZERO;
    gdi_blend_op ColorOp      = GDI_BLEND_OP_ADD;
    gdi_blend    SrcAlpha     = GDI_BLEND_ONE;
    gdi_blend    DstAlpha     = GDI_BLEND_ZERO;
    gdi_blend_op AlphaOp      = GDI_BLEND_OP_ADD;
};

struct gdi_graphics_pipeline_state {
    span<gdi_vtx_buffer_binding> VtxBufferBindings;
    gdi_topology                 Topology = GDI_TOPOLOGY_TRIANGLE_LIST;
    gdi_depth_state              DepthState;
    span<gdi_blend_state>        BlendStates = {{}};
};

struct gdi_graphics_pipeline_create_info {
    gdi_shader                              VS;
    gdi_shader                              PS;
    span<gdi_handle<gdi_bind_group_layout>> Layouts;
    gdi_graphics_pipeline_state             GraphicsState;
    gdi_handle<gdi_render_pass>             RenderPass;
};

enum gdi_bind_group_type {
    GDI_BIND_GROUP_TYPE_CONSTANT,
    GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
    GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
    GDI_BIND_GROUP_TYPE_SAMPLER,
    GDI_BIND_GROUP_TYPE_COUNT
};

bool GDI_Is_Bind_Group_Buffer(gdi_bind_group_type Type);
bool GDI_Is_Bind_Group_Dynamic(gdi_bind_group_type Type);
bool GDI_Is_Bind_Group_Texture(gdi_bind_group_type Type);

enum {
    GDI_SHADER_STAGE_NONE       = 0,
    GDI_SHADER_STAGE_VERTEX_BIT = (1 << 0),
    GDI_SHADER_STAGE_PIXEL_BIT  = (1 << 1),
};
typedef u32 gdi_shader_stage_flags;

struct gdi_bind_group_layout_binding {
    gdi_bind_group_type           Type;
    gdi_shader_stage_flags        StageFlags;
    span<gdi_handle<gdi_sampler>> ImmutableSamplers;
};

struct gdi_bind_group_layout_create_info {
    span<gdi_bind_group_layout_binding> Bindings;
};

struct gdi_bind_group_buffer {
    gdi_handle<gdi_buffer> Buffer;
    uptr                   Size = (uptr)-1;
};

struct gdi_bind_group_texture {
    gdi_handle<gdi_texture_view> TextureView;
};

struct gdi_bind_group_binding {
    gdi_bind_group_type    Type;
    gdi_bind_group_buffer  BufferBinding;
    gdi_bind_group_texture TextureBinding;
};

struct gdi_bind_group_write_info {
    span<gdi_bind_group_binding> Bindings;
};

struct gdi_bind_group_create_info {
    gdi_handle<gdi_bind_group_layout> Layout;
    gdi_bind_group_write_info         WriteInfo;
};

struct gdi_framebuffer_create_info {
    span<gdi_handle<gdi_texture_view>> Attachments;
    gdi_handle<gdi_render_pass> RenderPass;
};

enum class gdi_render_pass_attachment_type {
    Color,
    Depth
};

enum gdi_load_op {
    GDI_LOAD_OP_LOAD,
    GDI_LOAD_OP_CLEAR,
    GDI_LOAD_OP_COUNT
};

enum gdi_store_op {
    GDI_STORE_OP_STORE,
    GDI_STORE_OP_DONT_CARE,
    GDI_STORE_OP_COUNT
};

struct gdi_render_pass_attachment {
    gdi_render_pass_attachment_type Type;
    gdi_format                      Format;
    gdi_load_op                     LoadOp;
    gdi_store_op                    StoreOp;
    static gdi_render_pass_attachment Color(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp);
    static gdi_render_pass_attachment Depth(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp);
};

struct gdi_render_pass_create_info {
    span<gdi_render_pass_attachment> Attachments;
};

enum gdi_filter {
    GDI_FILTER_NEAREST,
    GDI_FILTER_LINEAR,
    GDI_FILTER_COUNT
};

enum gdi_address_mode {
    GDI_ADDRESS_MODE_CLAMP,
    GDI_ADDRESS_MODE_REPEAT,
    GDI_ADDRESS_MODE_COUNT
};

struct gdi_sampler_create_info {
    gdi_filter       Filter       = GDI_FILTER_NEAREST;
    gdi_address_mode AddressModeU = GDI_ADDRESS_MODE_CLAMP;
    gdi_address_mode AddressModeV = GDI_ADDRESS_MODE_CLAMP;
};

struct gdi_texture_view_create_info {
    gdi_handle<gdi_texture> Texture = {};
    gdi_format              Format  = GDI_FORMAT_NONE;
};


enum {
    GDI_TEXTURE_USAGE_FLAG_NONE,
    GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT = (1 << 0),
    GDI_TEXTURE_USAGE_FLAG_DEPTH_ATTACHMENT_BIT = (1 << 1),
    GDI_TEXTURE_USAGE_FLAG_SAMPLED_BIT = (1 << 2),
    GDI_TEXTURE_USAGE_FLAG_COPIED_BIT = (1 << 3)
};
typedef u32 gdi_texture_usage_flags;

struct gdi_texture_create_info {
    gdi_format              Format;
    u32                     Width;
    u32                     Height;
    gdi_texture_usage_flags UsageFlags;
    const_buffer            InitialData;
};

struct gdi_texture_upload {
    const_buffer Texels;
    u32          XOffset;
    u32          YOffset;
    u32          Width;
    u32          Height;          
};

enum {
    GDI_BUFFER_USAGE_FLAG_NONE,
    GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT       = (1 << 0),
    GDI_BUFFER_USAGE_FLAG_IDX_BUFFER_BIT       = (1 << 1),
    GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT  = (1 << 2),
    GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT   = (1 << 3),
    GDI_BUFFER_USAGE_FLAG_GPU_LOCAL_BUFFER_BIT = (1 << 4)
};
typedef u32 gdi_buffer_usage_flags;

struct gdi_buffer_create_info {
    uptr                   ByteSize;
    gdi_buffer_usage_flags UsageFlags;
    const_buffer           InitialData;
};

#if defined(OS_WIN32)
struct gdi_win32_window_data {
    HWND      Window;
    HINSTANCE Instance;
};
#endif

struct gdi_window_data {
#if defined(OS_WIN32)
    gdi_win32_window_data Win32;
#elif defined(OS_ANDROID)
#error "Not Implemented"
#elif defined(OS_OSX)
#error "Not Implemented"
#else
#error "Not Implemented"
#endif
};

struct gdi_swapchain_create_info {
    gdi_window_data         WindowData;
    gdi_format              TargetFormat;
    gdi_texture_usage_flags UsageFlags;
};


enum gdi_cmd_list_type {
    Graphics,
    Compute
};

struct gdi_device {
    string Name;
};

#define GDI_LOG_DEFINE(name) void name(gdi* GDI, string Message)
typedef GDI_LOG_DEFINE(gdi_log_func);

struct gdi_version {
    u32 Major = 1;
    u32 Minor = 0;
    u32 Patch = 0;
};

struct gdi_app_info {
    string      Name;
    gdi_version Version;
};

struct gdi_logging_callbacks {
    gdi_log_func* LogDebug;
    gdi_log_func* LogInfo;
    gdi_log_func* LogWarning;
    gdi_log_func* LogError;
};

struct gdi_create_info {
    gdi_logging_callbacks LoggingCallbacks;
    gdi_app_info          EngineInfo;
    gdi_app_info          AppInfo;
};

struct gdi_context_info {
    uptr ConstantBufferAlignment;
};

enum gdi_execute_status {
    GDI_EXECUTE_STATUS_NONE,
    GDI_EXECUTE_STATUS_RESIZE,
    GDI_EXECUTE_STATUS_ERROR
};

struct gdi_context_create_info {
    u32 DeviceIndex          = 0;
    u32 FrameCount           = 3;    
    u32 PipelineCount        = 128;
    u32 BindGroupCount       = 512;
    u32 BindGroupLayoutCount = 512;
    u32 FramebufferCount     = 128;
    u32 RenderPassCount      = 128;
    u32 SamplerCount         = 128;
    u32 TextureViewCount     = 1024;
    u32 TextureCount         = 1024;
    u32 BufferCount          = 1024;
    u32 SwapchainCount       = 32;
};

gdi*              GDI_Create(const gdi_create_info& CreateInfo);
void              GDI_Delete(gdi* GDI);
u32               GDI_Get_Device_Count(gdi* GDI);
void              GDI_Get_Device(gdi* GDI, gdi_device* Device, u32 DeviceIndex);
gdi_context*      GDI_Create_Context(gdi* GDI, const gdi_context_create_info& CreateInfo);

void                              GDI_Context_Delete(gdi_context* Context);
array<gdi_format>                 GDI_Context_Supported_Window_Formats(gdi_context* Context, const gdi_window_data& WindowData, arena* Arena);
const gdi_context_info*           GDI_Context_Get_Info(gdi_context* Context);

gdi_handle<gdi_pipeline>          GDI_Context_Create_Graphics_Pipeline(gdi_context* Context, const gdi_graphics_pipeline_create_info& CreateInfo);
void                              GDI_Context_Delete_Pipeline(gdi_context* Context, gdi_handle<gdi_pipeline> Handle);

gdi_handle<gdi_bind_group>        GDI_Context_Create_Bind_Group(gdi_context* Context, const gdi_bind_group_create_info& CreateInfo);
void                              GDI_Context_Delete_Bind_Group(gdi_context* Context, gdi_handle<gdi_bind_group> BindGroup);
bool                              GDI_Context_Write_Bind_Group(gdi_context* Context, gdi_handle<gdi_bind_group> BindGroup, const gdi_bind_group_write_info& WriteInfo);
gdi_handle<gdi_bind_group_layout> GDI_Context_Create_Bind_Group_Layout(gdi_context* Context, const gdi_bind_group_layout_create_info& CreateInfo);
void                              GDI_Context_Delete_Bind_Group_Layout(gdi_context* Context, gdi_handle<gdi_bind_group_layout> Handle);

gdi_handle<gdi_framebuffer>       GDI_Context_Create_Framebuffer(gdi_context* Context, const gdi_framebuffer_create_info& CreateInfo);
void                              GDI_Context_Delete_Framebuffer(gdi_context* Context, gdi_handle<gdi_framebuffer> Handle);
gdi_handle<gdi_render_pass>       GDI_Context_Create_Render_Pass(gdi_context* Context, const gdi_render_pass_create_info& CreateInfo);
void                              GDI_Context_Delete_Render_Pass(gdi_context* Context, gdi_handle<gdi_render_pass> Handle);

gdi_handle<gdi_sampler>      GDI_Context_Create_Sampler(gdi_context* Context, const gdi_sampler_create_info& CreateInfo);
void                         GDI_Context_Delete_Sampler(gdi_context* Context, gdi_handle<gdi_sampler> Sampler);
gdi_handle<gdi_texture_view> GDI_Context_Create_Texture_View(gdi_context* Context, const gdi_texture_view_create_info& CreateInfo);
void                         GDI_Context_Delete_Texture_View(gdi_context* Context, gdi_handle<gdi_texture_view> Handle);
gdi_handle<gdi_texture>      GDI_Context_Create_Texture(gdi_context* Context, const gdi_texture_create_info& CreateInfo);
void                         GDI_Context_Delete_Texture(gdi_context* Context, gdi_handle<gdi_texture> Handle);
void                         GDI_Context_Upload_Texture(gdi_context* Context, gdi_handle<gdi_texture> Handle, span<gdi_texture_upload> Uploads);

gdi_handle<gdi_buffer>            GDI_Context_Create_Buffer(gdi_context* Context, const gdi_buffer_create_info& CreateInfo);
u8*                               GDI_Context_Buffer_Map(gdi_context* Context, gdi_handle<gdi_buffer> Handle);
void                              GDI_Context_Buffer_Unmap(gdi_context* Context, gdi_handle<gdi_buffer> Handle);
void                              GDI_Context_Delete_Buffer(gdi_context* Context, gdi_handle<gdi_buffer> Buffer);

gdi_handle<gdi_swapchain>         GDI_Context_Create_Swapchain(gdi_context* Context, const gdi_swapchain_create_info& CreateInfo);
void                              GDI_Context_Delete_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle);
bool                              GDI_Context_Resize_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle);
span<gdi_handle<gdi_texture>>     GDI_Context_Get_Swapchain_Textures(gdi_context* Context, gdi_handle<gdi_swapchain> Handle);

gdi_cmd_list*                     GDI_Context_Begin_Cmd_List(gdi_context* Context, gdi_cmd_list_type Type, gdi_handle<gdi_swapchain> Swapchain = {});
gdi_execute_status                GDI_Context_Execute(gdi_context* Context);

enum gdi_resource_state {
    GDI_RESOURCE_STATE_NONE,
    GDI_RESOURCE_STATE_PRESENT,
    GDI_RESOURCE_STATE_COLOR,
    GDI_RESOURCE_STATE_DEPTH_WRITE,
    GDI_RESOURCE_STATE_COUNT
};

enum class gdi_resource_type {
    Buffer,
    Texture
};

struct gdi_resource {
    gdi_resource_type Type;
    union {
        gdi_handle<gdi_buffer>  BufferHandle = {};
        gdi_handle<gdi_texture> TextureHandle;
    };

    static gdi_resource Texture(gdi_handle<gdi_texture> Texture);
    static gdi_resource Buffer(gdi_handle<gdi_buffer> Buffer);
};

struct gdi_barrier {
    gdi_resource       Resource;
    gdi_resource_state OldState;
    gdi_resource_state NewState;
};

enum class gdi_clear_type {
    Color,
    Depth
};

union gdi_clear_color {
    f32 F32[4];
    s32 S32[4];
    u32 U32[4];
};

struct gdi_clear_depth {
    f32 Depth;
};

struct gdi_clear {
    gdi_clear_type Type;
    union {
        gdi_clear_color ClearColor;
        gdi_clear_depth ClearDepth;
    };

    static gdi_clear Color(f32 r, f32 g, f32 b, f32 a);
    static gdi_clear Depth(f32 Depth);
};

struct gdi_render_pass_begin_info {
    gdi_handle<gdi_render_pass> RenderPass;
    gdi_handle<gdi_framebuffer> Framebuffer;
    span<gdi_clear> ClearValues;
};

u32  GDI_Cmd_List_Get_Swapchain_Texture_Index(gdi_cmd_list* CmdList, gdi_resource_state ResourceState);
void GDI_Cmd_List_Barrier(gdi_cmd_list* CmdList, span<gdi_barrier> Barriers);
void GDI_Cmd_List_Begin_Render_Pass(gdi_cmd_list* CmdList, const gdi_render_pass_begin_info& BeginInfo);
void GDI_Cmd_List_End_Render_Pass(gdi_cmd_list* CmdList);
void GDI_Cmd_List_Set_Vtx_Buffers(gdi_cmd_list* CmdList, span<gdi_handle<gdi_buffer>> VtxBuffers);
void GDI_Cmd_List_Set_Idx_Buffer(gdi_cmd_list* CmdList, gdi_handle<gdi_buffer> IdxBuffer, gdi_format Format);
void GDI_Cmd_List_Set_Pipeline(gdi_cmd_list* CmdList, gdi_handle<gdi_pipeline> Pipeline);
void GDI_Cmd_List_Set_Bind_Groups(gdi_cmd_list* CmdList, u32 StartingIndex, span<gdi_handle<gdi_bind_group>> BindGroups);
void GDI_Cmd_List_Set_Dynamic_Bind_Groups(gdi_cmd_list* CmdList, u32 StartingIndex, span<gdi_handle<gdi_bind_group>> BindGroups, span<uptr> Offsets);
void GDI_Cmd_List_Draw_Instance(gdi_cmd_list* CmdList, u32 VtxCount, u32 InstanceCount, u32 VtxOffset, u32 InstanceOffset);
void GDI_Cmd_List_Draw_Indexed_Instance(gdi_cmd_list* CmdList, u32 IdxCount, u32 IdxOffset, u32 VtxOffset, u32 InstanceCount, u32 InstanceOffset);

#endif