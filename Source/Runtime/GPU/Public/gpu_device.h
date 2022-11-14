#ifndef GPU_DEVICE_H
#define GPU_DEVICE_H

typedef struct gpu_display_manager  gpu_display_manager;
typedef struct gpu_resource_manager gpu_resource_manager;
typedef struct gpu_device_context   gpu_device_context;
typedef struct gpu_cmd_buffer       gpu_cmd_buffer;

#define GPU_GET_DISPLAY_MANAGER(name) gpu_display_manager* name(gpu_device_context* _DeviceContext)
#define GPU_GET_RESOURCE_MANAGER(name) gpu_resource_manager* name(gpu_device_context* _DeviceContext)
#define GPU_ALLOCATE_CMD_BUFFER(name) gpu_cmd_buffer* name(gpu_device_context* _DeviceContext)
#define GPU_FREE_CMD_BUFFER(name) void name(gpu_device_context* _DeviceContext, gpu_cmd_buffer* _CmdBuffer)
#define GPU_DISPATCH_CMDS(name) void name(gpu_device_context* _DeviceContext, const gpu_cmd_buffer* const* CmdBuffers, uint32_t Count)

typedef GPU_GET_DISPLAY_MANAGER(gpu_get_display_manager);
typedef GPU_GET_RESOURCE_MANAGER(gpu_get_resource_manager);
typedef GPU_ALLOCATE_CMD_BUFFER(gpu_allocate_cmd_buffer);
typedef GPU_FREE_CMD_BUFFER(gpu_free_cmd_buffer);
typedef GPU_DISPATCH_CMDS(gpu_dispatch_cmds);

#define GPU_Get_Display_Manager(Context) (Context)->_VTable->Get_Display_Manager(Context)
#define GPU_Get_Resource_Manager(Context) (Context)->_VTable->Get_Resource_Manager(Context)
#define GPU_Allocate_Cmd_Buffer(Context) (Context)->_VTable->Allocate_Cmd_Buffer(Context)
#define GPU_Free_Cmd_Buffer(Context, CmdBuffer) (Context)->_VTable->Free_Cmd_Buffer(Context, CmdBuffer)
#define GPU_Dispatch_Cmds(Context, CmdBuffers, CmdBufferCount) (Context)->_VTable->Dispatch_Cmds(Context, CmdBuffers, CmdBufferCount)

typedef struct gpu_device_context_vtable
{
    gpu_get_display_manager*  Get_Display_Manager;
    gpu_get_resource_manager* Get_Resource_Manager;
    gpu_allocate_cmd_buffer*  Allocate_Cmd_Buffer;
    gpu_free_cmd_buffer*      Free_Cmd_Buffer;
    gpu_dispatch_cmds*        Dispatch_Cmds;
} gpu_device_context_vtable;

typedef struct gpu_device
{
    str8 DeviceName;
} gpu_device;

typedef struct gpu_device_list
{
    uint64_t     Count;
    gpu_device** Devices;
} gpu_device_list;

typedef struct gpu_device_context
{
    gpu_device* Device;
    gpu_device_context_vtable* _VTable;
} gpu_device_context;

#define GPU_SET_DEVICE(name) gpu_device_context* name(gpu_context* _Context, gpu_device* _Device)
typedef GPU_SET_DEVICE(gpu_set_device);

#endif