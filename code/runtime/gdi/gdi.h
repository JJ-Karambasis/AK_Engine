#ifndef GDI_H
#define GDI_H

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

struct gdi_context_create_info {
};

gdi*         GDI_Create(const gdi_create_info& CreateInfo);
void         GDI_Delete(gdi* GDI);
u32          GDI_Get_Device_Count(gdi* GDI);
void         GDI_Get_Device(gdi* GDI, gdi_device* Device, u32 DeviceIndex);
gdi_context* GDI_Create_Context(gdi* GDI, const gdi_context_create_info& CreateInfo);

#endif