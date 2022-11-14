#ifndef GPU_H
#define GPU_H

typedef struct gpu_context gpu_context;

#include "Public/gpu_device.h"
#include "Public/gpu_resource.h"
#include "Public/gpu_display.h"
#include "Public/gpu_cmd_buffer.h"

typedef struct gpu_context
{
    gpu_device_list DeviceList;
}  gpu_context;

#define GPU_INIT(name) gpu_context* name(core* Core)
#define GPU_SHUTDOWN(name) void name(gpu_context* Context)
#define GPU_RELOAD(name) void name(core* Core, gpu_context* Context)

typedef GPU_INIT(gpu_init);
typedef GPU_SHUTDOWN(gpu_shutdown);
typedef GPU_RELOAD(gpu_reload);

#endif