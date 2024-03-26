#ifndef GDI_H
#define GDI_H

enum gdi_format {
    GDI_FORMAT_NONE,
    GDI_FORMAT_R8G8_UNORM,
    GDI_FORMAT_R8G8B8_UNORM,
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

enum {
    GDI_TEXTURE_USAGE_FLAG_NONE,
    GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT = (1 << 0),
    GDI_TEXTURE_USAGE_FLAG_DEPTH_ATTACHMENT_BIT = (1 << 1),
    GDI_TEXTURE_USAGE_FLAG_SAMPLED_BIT = (1 << 2)
};
typedef uint32_t gdi_texture_usage_flags;

typedef uint64_t gdi_texture;
typedef uint64_t gdi_swapchain;

struct gdi;
struct gdi_context;

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
    gdi_app_info          AppInfo;
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

struct gdi_context_create_info {
    u32 DeviceIndex             = 0;
    ak_job_queue* ResourceQueue = nullptr;
    u32 TextureCount            = 1024;
    u32 SwapchainCount          = 32;
};

gdi*              GDI_Create(const gdi_create_info& CreateInfo);
void              GDI_Delete(gdi* GDI);
u32               GDI_Get_Device_Count(gdi* GDI);
void              GDI_Get_Device(gdi* GDI, gdi_device* Device, u32 DeviceIndex);
gdi_context*      GDI_Create_Context(gdi* GDI, const gdi_context_create_info& CreateInfo);

void              GDI_Context_Delete(gdi_context* Context);
array<gdi_format> GDI_Context_Supported_Window_Formats(gdi_context* Context, const gdi_window_data& WindowData, arena* Arena);
gdi_swapchain     GDI_Context_Create_Swapchain(gdi_context* Context, const gdi_swapchain_create_info& CreateInfo);
void              GDI_Context_Delete_Swapchain(gdi_context* Context, gdi_swapchain Swapchain);
void              GDI_Context_Resize_Swapchain(gdi_context* Context, gdi_swapchain Swapchain);

#endif