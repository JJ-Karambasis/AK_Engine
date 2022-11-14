#ifndef GPU_DISPLAY_H
#define GPU_DISPLAY_H

typedef struct gpu_display gpu_display;

#define GPU_DISPLAY_RESIZE(name) void name(gpu_display* _Display, uint32_t Width, uint32_t Height)
typedef GPU_DISPLAY_RESIZE(gpu_display_resize);
#define GPU_Display_Resize(Display, Width, Height) (Display)->_VTable->Resize(Display, Width, Height)

typedef struct gpu_display_vtable
{
    gpu_display_resize* Resize;
} gpu_display_vtable;

typedef struct gpu_display 
{
    gpu_display_vtable* _VTable;
} gpu_display;

#ifdef OS_WIN32
#define GPU_CREATE_DISPLAY(name) gpu_display* name(gpu_display_manager* _DisplayManager, HWND Window)
#else
#error Not Implemented
#endif
#define GPU_DELETE_DISPLAY(name) void name(gpu_display_manager* _DisplayManager, gpu_display* _Display)
#define GPU_PRESENT_DISPLAYS(name) void name(gpu_display_manager* _DisplayManager, const gpu_display* const* Displays, uint32_t Count)

typedef GPU_CREATE_DISPLAY(gpu_create_display);
typedef GPU_DELETE_DISPLAY(gpu_delete_display);
typedef GPU_PRESENT_DISPLAYS(gpu_present_displays);

#ifdef OS_WIN32
#define GPU_Create_Display(DisplayManager, Window) DisplayManager->_VTable->Create_Display(DisplayManager, Window)
#else
#error Not Implemented
#endif
#define GPU_Delete_Display(DisplayManager, Display) DisplayManager->_VTable->Delete_Display(DisplayManager, Display)
#define GPU_Present_Displays(DisplayManager, Displays, Count) DisplayManager->_VTable->Present_Displays(DisplayManager, Displays, Count)

typedef struct gpu_display_manager_vtable
{
    gpu_create_display*   Create_Display;
    gpu_delete_display*   Delete_Display;
    gpu_present_displays* Present_Displays;
} gpu_display_manager_vtable;

typedef struct gpu_display_manager
{
    gpu_display_manager_vtable* _VTable;
} gpu_display_manager;

#endif