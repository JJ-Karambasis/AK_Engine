#include <stdbool.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
#define VK_Load_Global_Func(object, name) object->name = (PFN_##name)vkGetInstanceProcAddr(NULL, #name)
#define VK_Load_Instance_Func(object, name) object->name = (PFN_##name)vkGetInstanceProcAddr(Instance, #name)
#define VK_Load_Device_Func(object, name) object->name = (PFN_##name)vkGetDeviceProcAddr(Device, #name)

#include "vk_loader.h"
#include "vk_shared_loader.c"

typedef struct {
    vk_shared_loader Base;
    HMODULE          Library;
} vk_win32_loader;

void VK_Shared_Loader_Load_Instance_Platform_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    if(!vkGetDeviceProcAddr) {
        vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(Instance, "vkGetDeviceProcAddr");
    }   
}

VKLOADERDEF vk_loader* VK_Get_Loader() {
    static vk_win32_loader Result;
    if(!Result.Base.Base.VTable) {
        Result.Base.Base.VTable = &G_VKSharedLoaderVTable;
    }

    if(!Result.Library) {
        Result.Library = LoadLibraryA("vulkan-1.dll");
        if(!Result.Library) {
            return NULL;
        }

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(Result.Library, "vkGetInstanceProcAddr");
    }

    return (vk_loader*)&Result;
}