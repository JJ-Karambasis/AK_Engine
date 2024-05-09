typedef struct {
    vk_loader Base;
    vk_global_funcs   GlobalFuncs;
    vk_instance_funcs InstanceFuncs;
    vk_device_funcs   DeviceFuncs;
    bool              HasLoadedGlobals;
    bool              HasLoadedInstances;
    bool              HasLoadedDevices;
} vk_shared_loader;

void VK_Shared_Loader_Load_Instance_Platform_Funcs(vk_shared_loader* Loader, VkInstance Instance);

void VK_Shared_Loader_Load_Instance_Default_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    if(!Loader->HasLoadedInstances) {
        vk_instance_funcs* InstanceFuncs = &Loader->InstanceFuncs;
        VK_Load_Instance_Func(InstanceFuncs, vkEnumeratePhysicalDevices);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceQueueFamilyProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkEnumerateDeviceExtensionProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkCreateDevice);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceMemoryProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkGetDeviceQueue);
        VK_Load_Instance_Func(InstanceFuncs, vkDestroyInstance);
        Loader->HasLoadedInstances = true;
    }
    
    VK_Shared_Loader_Load_Instance_Platform_Funcs(Loader, Instance);
}

void VK_Shared_Loader_Load_Surface_KHR_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_khr_surface* SurfaceKHR = &Loader->InstanceFuncs.SurfaceKHR;
    if(!SurfaceKHR->Enabled) {
        VK_Load_Instance_Func(SurfaceKHR, vkDestroySurfaceKHR);
        VK_Load_Instance_Func(SurfaceKHR, vkGetPhysicalDeviceSurfaceSupportKHR);
        VK_Load_Instance_Func(SurfaceKHR, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        VK_Load_Instance_Func(SurfaceKHR, vkGetPhysicalDeviceSurfaceFormatsKHR);
        VK_Load_Instance_Func(SurfaceKHR, vkGetPhysicalDeviceSurfacePresentModesKHR);
        SurfaceKHR->Enabled = true;
    }
}

void VK_Shared_Loader_Load_Get_Physical_Device_Properties_2_KHR_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_khr_get_physical_device_properties2* GetPhysicalDeviceProperties2KHR = &Loader->InstanceFuncs.GetPhysicalDeviceProperties2KHR;
    if(GetPhysicalDeviceProperties2KHR->Enabled) {
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceFeatures2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceProperties2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceFormatProperties2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceImageFormatProperties2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceQueueFamilyProperties2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceMemoryProperties2KHR);
        VK_Load_Instance_Func(GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceSparseImageFormatProperties2KHR);
        GetPhysicalDeviceProperties2KHR->Enabled = true;
    }
}

void VK_Shared_Loader_Load_Debug_Report_EXT_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_ext_debug_report* DebugReportEXT = &Loader->InstanceFuncs.DebugReportEXT;
    if(!DebugReportEXT->Enabled) {
        VK_Load_Instance_Func(DebugReportEXT, vkCreateDebugReportCallbackEXT);
        VK_Load_Instance_Func(DebugReportEXT, vkDebugReportMessageEXT);
        VK_Load_Instance_Func(DebugReportEXT, vkDestroyDebugReportCallbackEXT);
        DebugReportEXT->Enabled = true;
    }
}

void VK_Shared_Loader_Load_Debug_Utils_EXT_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_ext_debug_utils* DebugUtilsEXT = &Loader->InstanceFuncs.DebugUtilsEXT;
    if(!DebugUtilsEXT->Enabled) {
        VK_Load_Instance_Func(DebugUtilsEXT, vkSetDebugUtilsObjectNameEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkSetDebugUtilsObjectTagEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkQueueBeginDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkQueueEndDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkQueueInsertDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkCmdBeginDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkCmdEndDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkCmdInsertDebugUtilsLabelEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkCreateDebugUtilsMessengerEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkDestroyDebugUtilsMessengerEXT);
        VK_Load_Instance_Func(DebugUtilsEXT, vkSubmitDebugUtilsMessageEXT);
        DebugUtilsEXT->Enabled = true;
    }
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
void VK_Shared_Loader_Load_Win32_Surface_KHR_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_khr_win32_surface* Win32SurfaceKHR = &Loader->InstanceFuncs.Win32SurfaceKHR;
    if(!Win32SurfaceKHR->Enabled) {
        VK_Load_Instance_Func(Win32SurfaceKHR, vkCreateWin32SurfaceKHR);
        Win32SurfaceKHR->Enabled = true;
    }
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
void VK_Shared_Loader_Load_Android_Surface_KHR_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_khr_android_surface* AndroidSurfaceKHR = &Loader->InstanceFuncs.AndroidSurfaceKHR;
    if(!AndroidSurfaceKHR->Enabled) {
        VK_Load_Instance_Func(AndroidSurfaceKHR, vkCreateAndroidSurfaceKHR);
        AndroidSurfaceKHR->Enabled = true;
    }
}
#elif defined(VK_USE_PLATFORM_METAL_EXT)
void VK_Shared_Loader_Load_Metal_Surface_EXT_Funcs(vk_shared_loader* Loader, VkInstance Instance) {
    vk_ext_metal_surface* MetalSurfaceEXT = &Loader->InstanceFuncs.MetalSurfaceEXT
    if(!MetalSurfaceEXT->Enabled) {
        VK_Load_Instance_Func(MetalSurfaceEXT, vkCreateMetalSurfaceEXT);
        MetalSurfaceEXT->Enabled = true;
    }
}
#else
#error Not Implemented
#endif

void VK_Shared_Loader_Load_Device_Default_Funcs(vk_shared_loader* Loader, VkInstance Instance, VkDevice Device) {
    if(!Loader->HasLoadedDevices) {
        vk_device_funcs* DeviceFuncs = &Loader->DeviceFuncs;
        VK_Load_Device_Func(DeviceFuncs, vkCreateCommandPool);
        VK_Load_Device_Func(DeviceFuncs, vkAllocateCommandBuffers);
        VK_Load_Device_Func(DeviceFuncs, vkCreateFence);
        VK_Load_Device_Func(DeviceFuncs, vkCreateSemaphore);
        VK_Load_Device_Func(DeviceFuncs, vkResetFences);
        VK_Load_Device_Func(DeviceFuncs, vkResetCommandPool);
        VK_Load_Device_Func(DeviceFuncs, vkBeginCommandBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkEndCommandBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkQueueSubmit);
        VK_Load_Device_Func(DeviceFuncs, vkGetFenceStatus);
        VK_Load_Device_Func(DeviceFuncs, vkWaitForFences);
        VK_Load_Device_Func(DeviceFuncs, vkCreateRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCreateImageView);
        VK_Load_Device_Func(DeviceFuncs, vkCreateFramebuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCmdPipelineBarrier);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBeginRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCmdEndRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCreateBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkGetBufferMemoryRequirements);
        VK_Load_Device_Func(DeviceFuncs, vkAllocateMemory);
        VK_Load_Device_Func(DeviceFuncs, vkBindBufferMemory);
        VK_Load_Device_Func(DeviceFuncs, vkMapMemory);
        VK_Load_Device_Func(DeviceFuncs, vkCmdCopyBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCreatePipelineLayout);
        VK_Load_Device_Func(DeviceFuncs, vkCreateShaderModule);
        VK_Load_Device_Func(DeviceFuncs, vkCreateGraphicsPipelines);
        VK_Load_Device_Func(DeviceFuncs, vkCmdSetViewport);
        VK_Load_Device_Func(DeviceFuncs, vkCmdSetScissor);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindPipeline);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindVertexBuffers);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindIndexBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCmdDrawIndexed);
        VK_Load_Device_Func(DeviceFuncs, vkCreateDescriptorSetLayout);
        VK_Load_Device_Func(DeviceFuncs, vkCreateDescriptorPool);
        VK_Load_Device_Func(DeviceFuncs, vkAllocateDescriptorSets);
        VK_Load_Device_Func(DeviceFuncs, vkUpdateDescriptorSets);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindDescriptorSets);
        VK_Load_Device_Func(DeviceFuncs, vkCreateImage);
        VK_Load_Device_Func(DeviceFuncs, vkGetImageMemoryRequirements);
        VK_Load_Device_Func(DeviceFuncs, vkBindImageMemory);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyShaderModule);
        VK_Load_Device_Func(DeviceFuncs, vkCreateSampler);
        VK_Load_Device_Func(DeviceFuncs, vkCmdCopyBufferToImage);
        VK_Load_Device_Func(DeviceFuncs, vkCmdDraw);
        VK_Load_Device_Func(DeviceFuncs, vkDeviceWaitIdle);
        VK_Load_Device_Func(DeviceFuncs, vkResetDescriptorPool);
        VK_Load_Device_Func(DeviceFuncs, vkDestroySampler);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyImageView);
        VK_Load_Device_Func(DeviceFuncs, vkFreeMemory);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyImage);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyFramebuffer);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyDevice);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyCommandPool);
        VK_Load_Device_Func(DeviceFuncs, vkDestroySemaphore);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyPipelineLayout);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyPipeline);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkUnmapMemory);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyDescriptorSetLayout);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyDescriptorPool);
        VK_Load_Device_Func(DeviceFuncs, vkFreeDescriptorSets);
        VK_Load_Device_Func(DeviceFuncs, vkDestroyFence);
        VK_Load_Device_Func(DeviceFuncs, vkCmdExecuteCommands);
        Loader->HasLoadedDevices = true;
    }
}

void VK_Shared_Loader_Load_Swapchain_KHR_Funcs(vk_shared_loader* Loader, VkInstance Instance, VkDevice Device) {
    vk_khr_swapchain* SwapchainKHR = &Loader->DeviceFuncs.SwapchainKHR;
    if(!SwapchainKHR->Enabled) {
        VK_Load_Device_Func(SwapchainKHR, vkCreateSwapchainKHR);
        VK_Load_Device_Func(SwapchainKHR, vkDestroySwapchainKHR);
        VK_Load_Device_Func(SwapchainKHR, vkGetSwapchainImagesKHR);
        VK_Load_Device_Func(SwapchainKHR, vkAcquireNextImageKHR);
        VK_Load_Device_Func(SwapchainKHR, vkQueuePresentKHR);
        SwapchainKHR->Enabled = true;
    }
}

VK_LOADER_LOAD_GLOBAL_FUNCS_DEFINE(VK_Shared_Loader_Load_Global_Funcs) {
    vk_shared_loader* Loader = (vk_shared_loader*)_Loader;
    vk_global_funcs* GlobalFuncs = &Loader->GlobalFuncs;
    if(!Loader->HasLoadedGlobals) {
        VK_Load_Global_Func(GlobalFuncs, vkEnumerateInstanceExtensionProperties);
        VK_Load_Global_Func(GlobalFuncs, vkEnumerateInstanceLayerProperties);
        VK_Load_Global_Func(GlobalFuncs, vkCreateInstance);
        Loader->HasLoadedGlobals = true;
    }
    return GlobalFuncs;
}

VK_LOADER_LOAD_INSTANCE_FUNCS_DEFINE(VK_Shared_Loader_Load_Instance_Funcs) {
    vk_shared_loader* Loader = (vk_shared_loader*)_Loader;
    
    if(!Loader->HasLoadedInstances) {
        VK_Shared_Loader_Load_Instance_Default_Funcs(Loader, Instance);
    }

    if(InstanceInfo->SurfaceKHR) {
        VK_Shared_Loader_Load_Surface_KHR_Funcs(Loader, Instance);
    }

    if(InstanceInfo->GetPhysicalDeviceProperties2KHR) {
        VK_Shared_Loader_Load_Get_Physical_Device_Properties_2_KHR_Funcs(Loader, Instance);
    }

    if(InstanceInfo->DebugReportEXT) {
        VK_Shared_Loader_Load_Debug_Utils_EXT_Funcs(Loader, Instance);
    }

    if(InstanceInfo->DebugUtilsEXT) {
        VK_Shared_Loader_Load_Debug_Utils_EXT_Funcs(Loader, Instance);
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if(InstanceInfo->Win32SurfaceKHR) {
        VK_Shared_Loader_Load_Win32_Surface_KHR_Funcs(Loader, Instance);
    }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    if(InstanceInfo.->AndroidSurfaceKHR) {
        VK_Shared_Loader_Load_Android_Surface_KHR_Funcs(Loader, Instance);
    }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    if(InstanceInfo->MetalSurfaceEXT) {
        VK_Shared_Loader_Load_Metal_Surface_EXT_Funcs(Loader, Instance);
    }
#else
#error Not Implemented
#endif

    return &Loader->InstanceFuncs;
}

VK_LOADER_LOAD_DEVICE_FUNCS_DEFINE(VK_Shared_Loader_Load_Device_Funcs) {
    vk_shared_loader* Loader = (vk_shared_loader*)_Loader;
    if(!Loader->HasLoadedDevices) {
        VK_Shared_Loader_Load_Device_Default_Funcs(Loader, Instance, Device);
    }

    if(DeviceInfo->SwapchainKHR) {
        VK_Shared_Loader_Load_Swapchain_KHR_Funcs(Loader, Instance, Device);
    }

    return &Loader->DeviceFuncs;
}

static vk_loader_vtable G_VKSharedLoaderVTable = {
    VK_Shared_Loader_Load_Global_Funcs,
    VK_Shared_Loader_Load_Instance_Funcs,
    VK_Shared_Loader_Load_Device_Funcs
};