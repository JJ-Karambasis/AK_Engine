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

internal VkSurfaceKHR VK_Create_Surface(gdi* GDI, const gdi_window_data* WindowData) {
    const vk_instance_funcs* InstanceFuncs = GDI->InstanceFuncs;

    VkSurfaceKHR Surface = VK_NULL_HANDLE;

#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    const gdi_win32_window_data* Win32 = &WindowData->Win32;

    VkWin32SurfaceCreateInfoKHR CreateInfo = {};
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

internal bool VK_Create_Swapchain(gdi_context* Context, vk_swapchain* Swapchain, const gdi_swapchain_create_info& CreateInfo) {
    if(!Swapchain->Swapchain) {
        Zero_Struct(Swapchain);

        Swapchain->Surface = VK_Create_Surface(Context->GDI, &CreateInfo.WindowData);
        if(!Swapchain->Surface) {
            //todo: Diagnostic
            return false;
        }
    }

    VkSurfaceCapabilitiesKHR SurfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context->PhysicalDevice->Device, Swapchain->Surface, &SurfaceCaps);

    VkCompositeAlphaFlagBitsKHR CompositeAlphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    if (SurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        CompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if (SurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        CompositeAlphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else {
        //todo: Diagnostic
        return false;
    }

    //Max of 3 swapchain images
    u32 MinImageCount = Min(SurfaceCaps.maxImageCount, 3);
    if (MinImageCount < 2) {
        //todo: Diagnostic
        return false;
    }

    Swapchain->Format = CreateInfo.TargetFormat;
    Swapchain->UsageFlags = CreateInfo.UsageFlags;

    VkFormat TargetFormat = VK_Get_Format(Swapchain->Format);

    VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = Swapchain->Surface,
        .minImageCount = MinImageCount, 
        .imageFormat = TargetFormat,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = SurfaceCaps.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_Convert_To_Image_Usage_Flags(Swapchain->UsageFlags),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = CompositeAlphaFlags,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .oldSwapchain = Swapchain->Swapchain
    };

    VkSwapchainKHR SwapchainHandle;
    if(vkCreateSwapchainKHR(Context->Device, &SwapchainCreateInfo, Context->VKAllocator, &SwapchainHandle) != VK_SUCCESS) {
        //todo: Diagnostic
        return false;
    }

    Swapchain->Width = SurfaceCaps.currentExtent.width;
    Swapchain->Height = SurfaceCaps.currentExtent.height;
    Swapchain->Swapchain = SwapchainHandle;

    return true;
}

internal bool VK_Create_Swapchain_Textures(gdi_context* Context, vk_swapchain* Swapchain) {
    u32 ImageCount;
    vkGetSwapchainImagesKHR(Context->Device, Swapchain->Swapchain, &ImageCount, VK_NULL_HANDLE);

    Assert(ImageCount >= 2);
    
    scratch Scratch = Scratch_Get();
    VkImage* Images = Scratch_Push_Array(&Scratch, ImageCount, VkImage);
    vkGetSwapchainImagesKHR(Context->Device, Swapchain->Swapchain, &ImageCount, Images);

    Swapchain->Textures = array<gdi_handle<gdi_texture>>(Context->GDI->MainAllocator, ImageCount);
    Array_Resize(&Swapchain->Textures, ImageCount);

    for(u32 i = 0; i < ImageCount; i++) {
        async_handle<vk_texture> TextureHandle = VK_Resource_Manager_Allocate(&Context->TextureManager);
        if(TextureHandle.Is_Null()) {
            //todo: Diagnostic 
            return false;
        }

        pool_writer_lock TextureWriter(&Context->TextureManager.Pool, TextureHandle);
        TextureWriter->Image  = Images[i];
        TextureWriter->Width  = Swapchain->Width;
        TextureWriter->Height = Swapchain->Height;
        TextureWriter->Format = Swapchain->Format;
        TextureWriter.Unlock();

        Swapchain->Textures[i] = gdi_handle<gdi_texture>(TextureHandle.ID);
    }

    return true;
}

internal void VK_Delete_Swapchain(gdi_context* Context, vk_swapchain* Swapchain) {
    if(Swapchain->Swapchain) {
        vkDestroySwapchainKHR(Context->Device, Swapchain->Swapchain, Context->VKAllocator);
        Swapchain->Swapchain = VK_NULL_HANDLE;
    }

    if(Swapchain->Surface) {
        vkDestroySurfaceKHR(Context->GDI->Instance, Swapchain->Surface, Context->VKAllocator);
        Swapchain->Surface = VK_NULL_HANDLE;
    }
}

internal void VK_Delete_Swapchain_Textures(gdi_context* Context, vk_swapchain* Swapchain) {
    for(gdi_handle<gdi_texture>& Handle : Swapchain->Textures) {
        async_handle<vk_texture> TextureHandle(Handle.ID);
        VK_Resource_Manager_Free(&Context->TextureManager, TextureHandle);
        Handle = {};
    }
    Array_Free(&Swapchain->Textures);
}

internal void VK_Delete_Swapchain_Full(gdi_context* Context, vk_swapchain* Swapchain) {
    VK_Delete_Swapchain_Textures(Context, Swapchain);
    VK_Delete_Swapchain(Context, Swapchain);
}
