#ifndef GL_DEVICE_H
#define GL_DEVICE_H

typedef struct gl_context gl_context;
typedef struct gl_display_manager gl_display_manager;
typedef struct gl_resource_manager gl_resource_manager;
typedef struct gl_cmd_pool gl_cmd_pool;

typedef struct gl_device
{
    gpu_device  Device;
    gl_context* Context;
} gl_device;

typedef struct gl_device_context
{
    gpu_device_context   DeviceContext;
    struct gl_device*    Device;
    arena*               Arena;
    gl_display_manager*  DisplayManager;
    gl_resource_manager* ResourceManager;
    gl_cmd_pool*         CmdPool;
} gl_device_context;

GPU_GET_DISPLAY_MANAGER(GL_Device_Context_Get_Display_Manager);
GPU_GET_RESOURCE_MANAGER(GL_Device_Context_Get_Resource_Manager);
GPU_ALLOCATE_CMD_BUFFER(GL_Device_Context_Allocate_Cmd_Buffer);
GPU_FREE_CMD_BUFFER(GL_Device_Context_Free_Cmd_Buffer);
GPU_DISPATCH_CMDS(GL_Device_Context_Dispatch_Cmds);

#endif