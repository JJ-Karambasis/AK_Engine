internal string VK_Get_Surface_Extension_Name() {
#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    local_persist string SurfaceExtension = String_Lit(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    local_persist string SurfaceExtension = String_Lit(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    local_persist string SurfaceExtension = String_Lit(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#else
    #error Not Implemented
#endif

    return SurfaceExtension;
}

internal bool VK_Is_Surface_Extension(string ExtensionName) {
    return ExtensionName == VK_Get_Surface_Extension_Name();
}  

internal void VK_Set_Surface_Extension(vk_instance_extension_support* InstanceExtensions) {
#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    InstanceExtensions->Win32SurfaceKHR = true;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    InstanceExtensions->AndroidSurfaceKHR = true;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    InstanceExtensions->MetalSurfaceEXT = true;
#else
    #error Not Implemented
#endif
}

internal bool VK_Has_Surface_Extension(const vk_instance_extension_support* InstanceExtensions) {
#if  defined(VK_USE_PLATFORM_WIN32_KHR)
    return InstanceExtensions->Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    return InstanceExtensions.AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    return InstanceExtensions.MetalSurfaceEXT;
#else
    #error Not Implemented
    return false;
#endif
}

internal VkSurfaceKHR VK_Create_Surface(gdi* GDI, gdi_window_data* WindowData) {
    const vk_instance_funcs* InstanceFuncs = GDI->InstanceFuncs;

    VkSurfaceKHR Surface = VK_NULL_HANDLE;

#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    gdi_win32_window_data* Win32 = &WindowData->Win32;

    VkWin32SurfaceCreateInfoKHR CreateInfo = {0};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hinstance = Win32->Instance;
    CreateInfo.hwnd = Win32->Window;

    Assert(InstanceFuncs->Win32SurfaceKHR.Enabled);

    if (InstanceFuncs->Win32SurfaceKHR.vkCreateWin32SurfaceKHR(GDI->Instance, &CreateInfo, &GDI->VKAllocator, &Surface) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#error "Not Implemented"
#elif defined(VK_USE_PLATFORM_METAL_EXT)
#error "Not Implemented"
#else
#error "Not Implemented"
#endif

    return Surface;
}

internal VkSurfaceKHR VK_Create_Temp_Surface(gdi* GDI) {
    gdi_window_data WindowData = {};
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    WindowData.Win32.Instance = GetModuleHandleW(NULL);
    WindowData.Win32.Window   = GetDesktopWindow();
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)

#elif defined(VK_USE_PLATFORM_METAL_EXT)

#else
#error Not Implemented
#endif

    return VK_Create_Surface(GDI, &WindowData);
}