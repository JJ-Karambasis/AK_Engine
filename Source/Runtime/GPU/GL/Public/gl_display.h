#ifndef GL_DISPLAY_H
#define GL_DISPLAY_H

typedef struct gl_display
{
    gpu_display  Display;
    gl_context*  Context;
    gl_display*  Next;
} gl_display;

typedef struct gl_display_manager
{
    gpu_display_manager DisplayManager;
    gl_display*         FreeDisplays;
} gl_display_manager;

gl_display_manager* GL_Display_Manager_Create(gl* GL, allocator* Allocator);

#endif