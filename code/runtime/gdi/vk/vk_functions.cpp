void VK_Load_Global_Funcs(gdi* GDI) {
    GDI->GlobalFuncs = VK_Loader_Load_Global_Funcs(GDI->Loader);
    const vk_global_funcs* GlobalFuncs = GDI->GlobalFuncs; 

    vkEnumerateInstanceExtensionProperties = GlobalFuncs->vkEnumerateInstanceExtensionProperties;
    vkEnumerateInstanceLayerProperties = GlobalFuncs->vkEnumerateInstanceLayerProperties;
    vkCreateInstance = GlobalFuncs->vkCreateInstance;
}

void VK_Load_Instance_Funcs(gdi* GDI, vk_instance_extension_support* InstanceInfo) {
    GDI->InstanceFuncs = VK_Loader_Load_Instance_Funcs(GDI->Loader, GDI->Instance, InstanceInfo);
    const vk_instance_funcs* InstanceFuncs = GDI->InstanceFuncs;

    vkEnumeratePhysicalDevices = InstanceFuncs->vkEnumeratePhysicalDevices;
    vkGetPhysicalDeviceQueueFamilyProperties = InstanceFuncs->vkGetPhysicalDeviceQueueFamilyProperties;
    vkEnumerateDeviceExtensionProperties = InstanceFuncs->vkEnumerateDeviceExtensionProperties;
    vkCreateDevice = InstanceFuncs->vkCreateDevice;
    vkGetPhysicalDeviceProperties = InstanceFuncs->vkGetPhysicalDeviceProperties;
    vkGetPhysicalDeviceMemoryProperties = InstanceFuncs->vkGetPhysicalDeviceMemoryProperties;
    vkGetDeviceQueue = InstanceFuncs->vkGetDeviceQueue;
    vkDestroyInstance = InstanceFuncs->vkDestroyInstance;

    Assert(InstanceFuncs->SurfaceKHR.Enabled);
    vkGetPhysicalDeviceSurfaceSupportKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceSupportKHR;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    vkGetPhysicalDeviceSurfaceFormatsKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceFormatsKHR;
    vkDestroySurfaceKHR = InstanceFuncs->SurfaceKHR.vkDestroySurfaceKHR;
}

void VK_Load_Device_Funcs(gdi* GDI, gdi_context* Context) {
    VkDevice Device = Context->Device;
    vk_device_extension_support* DeviceInfo = &Context->PhysicalDevice->DeviceInfo;
    Context->DeviceFuncs = VK_Loader_Load_Device_Funcs(GDI->Loader, GDI->Instance, Device, DeviceInfo);

    const vk_device_funcs* DeviceFuncs = Context->DeviceFuncs;

    vkCreateCommandPool = DeviceFuncs->vkCreateCommandPool;
    vkAllocateCommandBuffers = DeviceFuncs->vkAllocateCommandBuffers;
    vkCreateFence = DeviceFuncs->vkCreateFence;
    vkCreateSemaphore = DeviceFuncs->vkCreateSemaphore;
    vkResetFences = DeviceFuncs->vkResetFences;
    vkResetCommandPool = DeviceFuncs->vkResetCommandPool;
    vkBeginCommandBuffer = DeviceFuncs->vkBeginCommandBuffer;
    vkEndCommandBuffer = DeviceFuncs->vkEndCommandBuffer;
    vkQueueSubmit = DeviceFuncs->vkQueueSubmit;
    vkGetFenceStatus = DeviceFuncs->vkGetFenceStatus;
    vkWaitForFences = DeviceFuncs->vkWaitForFences;
    vkCreateRenderPass = DeviceFuncs->vkCreateRenderPass;
    vkCreateImageView = DeviceFuncs->vkCreateImageView;
    vkCreateFramebuffer = DeviceFuncs->vkCreateFramebuffer;
    vkCmdPipelineBarrier = DeviceFuncs->vkCmdPipelineBarrier;
    vkCmdBeginRenderPass = DeviceFuncs->vkCmdBeginRenderPass;
    vkCmdEndRenderPass = DeviceFuncs->vkCmdEndRenderPass;
    vkCreateBuffer = DeviceFuncs->vkCreateBuffer;
    vkGetBufferMemoryRequirements = DeviceFuncs->vkGetBufferMemoryRequirements;
    vkAllocateMemory = DeviceFuncs->vkAllocateMemory;
    vkBindBufferMemory = DeviceFuncs->vkBindBufferMemory;
    vkMapMemory = DeviceFuncs->vkMapMemory;
    vkCmdCopyBuffer = DeviceFuncs->vkCmdCopyBuffer;
    vkCreatePipelineLayout = DeviceFuncs->vkCreatePipelineLayout;
    vkCreateShaderModule = DeviceFuncs->vkCreateShaderModule;
    vkCreateGraphicsPipelines = DeviceFuncs->vkCreateGraphicsPipelines;
    vkCmdSetViewport = DeviceFuncs->vkCmdSetViewport;
    vkCmdSetScissor = DeviceFuncs->vkCmdSetScissor;
    vkCmdBindPipeline = DeviceFuncs->vkCmdBindPipeline;
    vkCmdBindVertexBuffers = DeviceFuncs->vkCmdBindVertexBuffers;
    vkCmdBindIndexBuffer = DeviceFuncs->vkCmdBindIndexBuffer;
    vkCmdDrawIndexed = DeviceFuncs->vkCmdDrawIndexed;
    vkCreateDescriptorSetLayout = DeviceFuncs->vkCreateDescriptorSetLayout;
    vkCreateDescriptorPool = DeviceFuncs->vkCreateDescriptorPool;
    vkAllocateDescriptorSets = DeviceFuncs->vkAllocateDescriptorSets;
    vkUpdateDescriptorSets = DeviceFuncs->vkUpdateDescriptorSets;
    vkCmdBindDescriptorSets = DeviceFuncs->vkCmdBindDescriptorSets;
    vkCreateImage = DeviceFuncs->vkCreateImage;
    vkGetImageMemoryRequirements = DeviceFuncs->vkGetImageMemoryRequirements;
    vkBindImageMemory = DeviceFuncs->vkBindImageMemory;
    vkDestroyShaderModule = DeviceFuncs->vkDestroyShaderModule;
    vkCreateSampler = DeviceFuncs->vkCreateSampler;
    vkCmdCopyBufferToImage = DeviceFuncs->vkCmdCopyBufferToImage;
    vkCmdDraw = DeviceFuncs->vkCmdDraw;
    vkDeviceWaitIdle = DeviceFuncs->vkDeviceWaitIdle;
    vkResetDescriptorPool = DeviceFuncs->vkResetDescriptorPool;
    vkDestroySampler = DeviceFuncs->vkDestroySampler;
    vkDestroyImageView = DeviceFuncs->vkDestroyImageView;
    vkFreeMemory = DeviceFuncs->vkFreeMemory;
    vkDestroyImage = DeviceFuncs->vkDestroyImage;
    vkDestroyFramebuffer = DeviceFuncs->vkDestroyFramebuffer;
    vkDestroyDevice = DeviceFuncs->vkDestroyDevice;
    vkDestroyRenderPass = DeviceFuncs->vkDestroyRenderPass;
    vkDestroyCommandPool = DeviceFuncs->vkDestroyCommandPool;
    vkDestroySemaphore = DeviceFuncs->vkDestroySemaphore;
    vkDestroyPipelineLayout = DeviceFuncs->vkDestroyPipelineLayout;
    vkDestroyPipeline = DeviceFuncs->vkDestroyPipeline;
    vkDestroyBuffer = DeviceFuncs->vkDestroyBuffer;
    vkUnmapMemory = DeviceFuncs->vkUnmapMemory;
;
    Assert(DeviceFuncs->SwapchainKHR.Enabled);

    vkCreateSwapchainKHR = DeviceFuncs->SwapchainKHR.vkCreateSwapchainKHR;
    vkGetSwapchainImagesKHR = DeviceFuncs->SwapchainKHR.vkGetSwapchainImagesKHR;
    vkAcquireNextImageKHR = DeviceFuncs->SwapchainKHR.vkAcquireNextImageKHR;
    vkQueuePresentKHR = DeviceFuncs->SwapchainKHR.vkQueuePresentKHR;
    vkDestroySwapchainKHR = DeviceFuncs->SwapchainKHR.vkDestroySwapchainKHR;
}