bool Level_Editor_Init(level_editor* LevelEditor, const level_editor_create_info& CreateInfo) {
    LevelEditor->Heap = Heap_Create(Core_Get_Base_Allocator());
    LevelEditor->Arena = Arena_Create(LevelEditor->Heap);
    LevelEditor->GDIContext = CreateInfo.GDIContext;
    LevelEditor->WindowID = CreateInfo.WindowID;
    LevelEditor->Swapchain = OS_Window_Get_Swapchain(LevelEditor->WindowID);
    LevelEditor->UIRenderPass = CreateInfo.UIRenderPass;
    Array_Init(&LevelEditor->SwapchainViews, LevelEditor->Arena);
    Array_Init(&LevelEditor->SwapchainFramebuffers, LevelEditor->Arena);
    LevelEditor->IsOpen = true;

    Array_Push(CreateInfo.EventSubscribers, {
        .EventCallback = Level_Editor_OS_Event_Callback,
        .UserData = LevelEditor
    });

    UI_Init(&LevelEditor->UI, LevelEditor->Heap);

    return true;
}

bool Level_Editor_Is_Open(level_editor* LevelEditor) {
    return (bool)LevelEditor->IsOpen;
}

void Level_Editor_Update(level_editor* LevelEditor) {
    uvec2 CurrentWindowSize;
    OS_Window_Get_Resolution(LevelEditor->WindowID, &CurrentWindowSize.w, &CurrentWindowSize.h);

    if(CurrentWindowSize.x != 0 && CurrentWindowSize.y != 0 && Level_Editor_Is_Open(LevelEditor)) {
        if(CurrentWindowSize != LevelEditor->WindowSize) {
            if(LevelEditor->SwapchainFramebuffers.Count) {
                for(gdi_handle<gdi_framebuffer> Framebuffer : LevelEditor->SwapchainFramebuffers) {
                    GDI_Context_Delete_Framebuffer(LevelEditor->GDIContext, Framebuffer);
                }
                Array_Clear(&LevelEditor->SwapchainFramebuffers);
            }

            if(LevelEditor->SwapchainViews.Count) {
                for(gdi_handle<gdi_texture_view> SwapchainView : LevelEditor->SwapchainViews) {
                    GDI_Context_Delete_Texture_View(LevelEditor->GDIContext, SwapchainView);
                }
                Array_Clear(&LevelEditor->SwapchainViews);
            }

            GDI_Context_Resize_Swapchain(LevelEditor->GDIContext, LevelEditor->Swapchain);

            span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(LevelEditor->GDIContext, LevelEditor->Swapchain);
            for(uptr i = 0; i < Textures.Count; i++) {
                Array_Push(&LevelEditor->SwapchainViews, GDI_Context_Create_Texture_View(LevelEditor->GDIContext, {
                    .Texture = Textures[i]
                }));

                Array_Push(&LevelEditor->SwapchainFramebuffers, GDI_Context_Create_Framebuffer(LevelEditor->GDIContext, {
                    .Attachments = {LevelEditor->SwapchainViews[i]},
                    .RenderPass = LevelEditor->UIRenderPass
                }));
            }

            LevelEditor->WindowSize = CurrentWindowSize;
        }
    }
}

void Level_Editor_Render(level_editor* LevelEditor) {
    gdi_context* GDIContext = LevelEditor->GDIContext;
    span<gdi_handle<gdi_texture>> SwapchainTextures = GDI_Context_Get_Swapchain_Textures(GDIContext, LevelEditor->Swapchain);
    gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics, LevelEditor->Swapchain); 
    u32 TextureIndex = GDI_Cmd_List_Get_Swapchain_Texture_Index(CmdList, GDI_RESOURCE_STATE_COLOR);

    GDI_Cmd_List_Barrier(CmdList, {
        {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_NONE, GDI_RESOURCE_STATE_COLOR}, 
    });

    GDI_Cmd_List_Begin_Render_Pass(CmdList, {
        .RenderPass = LevelEditor->UIRenderPass,
        .Framebuffer = LevelEditor->SwapchainFramebuffers[TextureIndex],
        .ClearValues = { 
            gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
        }
    });

    GDI_Cmd_List_End_Render_Pass(CmdList);
    GDI_Cmd_List_Barrier(CmdList, {
        {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_COLOR, GDI_RESOURCE_STATE_PRESENT}
    });
}

void Level_Editor_Shutdown(level_editor* LevelEditor) {
    //TODO: Implement
}