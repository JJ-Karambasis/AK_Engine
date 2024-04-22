internal bool VK_Create_Pipeline(gdi_context* Context, vk_pipeline* Pipeline, const gdi_graphics_pipeline_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get();
    fixed_array<VkDescriptorSetLayout> SetLayouts(&Scratch, CreateInfo.Layouts.Count);
    for(uptr i = 0; i < CreateInfo.Layouts.Count; i++) {
        async_handle<vk_bind_group_layout> LayoutHandle(CreateInfo.Layouts[i].ID);
        vk_bind_group_layout* Layout = Async_Pool_Get(&Context->ResourceContext.BindGroupLayouts, LayoutHandle);
        if(!Layout) {
            Assert(false);
            return false;
        }

        SetLayouts[i] = Layout->SetLayout;
    }

    VkPipelineLayoutCreateInfo LayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = Safe_U32(SetLayouts.Count),
        .pSetLayouts = SetLayouts.Ptr
    };

    if(vkCreatePipelineLayout(Context->Device, &LayoutInfo, Context->VKAllocator, &Pipeline->Layout) != VK_SUCCESS) {
        //todo: Diagnostics
        return false;
    }

    VkShaderModule VtxShader, PxlShader;
    VkShaderModuleCreateInfo VtxShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = CreateInfo.VS.ByteCode.Size,
        .pCode = (const uint32_t*)CreateInfo.VS.ByteCode.Ptr
    };

    VkShaderModuleCreateInfo PxlShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = CreateInfo.PS.ByteCode.Size,
        .pCode = (const uint32_t*)CreateInfo.PS.ByteCode.Ptr
    };

    if(vkCreateShaderModule(Context->Device, &VtxShaderInfo, Context->VKAllocator, &VtxShader) != VK_SUCCESS) {
        //todo: Diagnostic
        return false;
    }

    if(vkCreateShaderModule(Context->Device, &PxlShaderInfo, Context->VKAllocator, &PxlShader) != VK_SUCCESS) {
        //todo: Diagnostic
        vkDestroyShaderModule(Context->Device, VtxShader, Context->VKAllocator);
        return false;
    }

    VkPipelineShaderStageCreateInfo Stages[] = { {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = VtxShader,
            .pName = CreateInfo.VS.EntryName.Str
        }, {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = PxlShader,
            .pName = CreateInfo.PS.EntryName.Str
        }
    };

    array<VkVertexInputBindingDescription> VtxBindings(&Scratch, CreateInfo.GraphicsState.VtxBufferBindings.Count);
    array<VkVertexInputAttributeDescription> VtxAttributes(&Scratch, CreateInfo.GraphicsState.VtxBufferBindings.Count*3);

    u32 BindingIndex = 0;
    u32 Location = 0;
    for(const gdi_vtx_buffer_binding& Binding : CreateInfo.GraphicsState.VtxBufferBindings) {
        Array_Push(&VtxBindings, {
            .binding = BindingIndex,
            .stride = Safe_U32(Binding.ByteStride),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        });

        for(const gdi_vtx_attribute& Attribute : Binding.Attributes) {
            Array_Push(&VtxAttributes, {
                .location = Location,
                .binding = BindingIndex,
                .format = VK_Get_Format(Attribute.Format),
                .offset = Safe_U32(Attribute.ByteOffset)
            });
            Location++;
        }
        BindingIndex++;
    }

    VkPipelineVertexInputStateCreateInfo VtxInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = Safe_U32(VtxBindings.Count),
        .pVertexBindingDescriptions = VtxBindings.Ptr,
        .vertexAttributeDescriptionCount = Safe_U32(VtxAttributes.Count),
        .pVertexAttributeDescriptions = VtxAttributes.Ptr
    };

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_Get_Topology(CreateInfo.GraphicsState.Topology)
    };

    VkViewport Viewport = {};
    VkRect2D Scissor = {};

    VkPipelineViewportStateCreateInfo ViewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &Viewport,
        .scissorCount = 1,
        .pScissors = &Scissor
    };

    VkPipelineRasterizationStateCreateInfo RasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo MultisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineDepthStencilStateCreateInfo DepthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = CreateInfo.GraphicsState.DepthState.DepthTestEnabled,
        .depthWriteEnable = CreateInfo.GraphicsState.DepthState.DepthWriteEnabled,
        .depthCompareOp = VK_Get_Compare_Op(CreateInfo.GraphicsState.DepthState.ComparisonFunc)
    };

    fixed_array<VkPipelineColorBlendAttachmentState> Attachments(&Scratch, CreateInfo.GraphicsState.BlendStates.Count);
    for(uptr i = 0; i < Attachments.Count; i++) {
        const gdi_blend_state* BlendState = &CreateInfo.GraphicsState.BlendStates[i];
        Attachments[i] = {
            .blendEnable         = BlendState->BlendEnabled,
            .srcColorBlendFactor = VK_Get_Blend_Factor(BlendState->SrcColor),
            .dstColorBlendFactor = VK_Get_Blend_Factor(BlendState->DstColor),
            .colorBlendOp        = VK_Get_Blend_Op(BlendState->ColorOp),
            .srcAlphaBlendFactor = VK_Get_Blend_Factor(BlendState->SrcAlpha),
            .dstAlphaBlendFactor = VK_Get_Blend_Factor(BlendState->DstAlpha),
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT 
        };
    }

    VkPipelineColorBlendStateCreateInfo ColorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = Safe_U32(Attachments.Count),
        .pAttachments = Attachments.Ptr
    };

    VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo DynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = Array_Count(DynamicStates),
        .pDynamicStates = DynamicStates
    };

    VkPipelineCreateFlags Flags = 0;
#ifdef DEBUG_BUILD
    Flags |= VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
#endif

    async_handle<vk_render_pass> RenderPassHandle(CreateInfo.RenderPass.ID);
    vk_render_pass* RenderPass = Async_Pool_Get(&Context->ResourceContext.RenderPasses, RenderPassHandle);
    if(!RenderPass) {
        //todo: Diagnostics
        return false;
    }

    VkGraphicsPipelineCreateInfo GraphicsPipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = Flags,
        .stageCount = Array_Count(Stages),
        .pStages = Stages,
        .pVertexInputState = &VtxInputState,
        .pInputAssemblyState = &InputAssemblyState,
        .pViewportState = &ViewportState,
        .pRasterizationState = &RasterizationState,
        .pMultisampleState = &MultisampleState,
        .pDepthStencilState = &DepthStencilState,
        .pColorBlendState = &ColorBlendState,
        .pDynamicState = &DynamicState,
        .layout = Pipeline->Layout,
        .renderPass = RenderPass->RenderPass
    };

    if(vkCreateGraphicsPipelines(Context->Device, VK_NULL_HANDLE, 1, &GraphicsPipelineInfo, Context->VKAllocator, &Pipeline->Pipeline) != VK_SUCCESS) {
        //todo: Diagnostics
        vkDestroyShaderModule(Context->Device, VtxShader, Context->VKAllocator);
        vkDestroyShaderModule(Context->Device, PxlShader, Context->VKAllocator);
        return false;
    }

    Pipeline->BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    vkDestroyShaderModule(Context->Device, VtxShader, Context->VKAllocator);
    vkDestroyShaderModule(Context->Device, PxlShader, Context->VKAllocator);
    return true;
}

internal void VK_Delete_Pipeline(gdi_context* Context, vk_pipeline* Pipeline) {
    if(Pipeline->Layout) {
        vkDestroyPipelineLayout(Context->Device, Pipeline->Layout, Context->VKAllocator);
        Pipeline->Layout = VK_NULL_HANDLE;
    }

    if(Pipeline->Pipeline) {
        vkDestroyPipeline(Context->Device, Pipeline->Pipeline, Context->VKAllocator);
        Pipeline->Pipeline = VK_NULL_HANDLE;
    }
}

internal void VK_Pipeline_Record_Frame(gdi_context* Context, async_handle<vk_pipeline> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.PipelinesInUse[Handle.Index()], true);
}
