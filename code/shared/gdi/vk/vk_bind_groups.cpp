internal bool VK_Create_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout, const gdi_bind_group_layout_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get(); 
    fixed_array<VkDescriptorSetLayoutBinding> Bindings(&Scratch, CreateInfo.Bindings.Count);

    for(uptr i = 0; i < CreateInfo.Bindings.Count; i++) {
        fixed_array<VkSampler> Samplers(&Scratch, CreateInfo.Bindings[i].ImmutableSamplers.Count);

        for(uptr j = 0; j < Samplers.Count; j++) {
            vk_handle<vk_sampler> SamplerHandle(CreateInfo.Bindings[i].ImmutableSamplers[j].ID);
            vk_sampler* Sampler = VK_Resource_Get(Context->ResourceContext.Samplers, SamplerHandle);
            if(!Sampler) {
                Assert(false);
                return false;
            }
            Samplers[j] = Sampler->Handle;
            Layout->Add_Reference(Context, Sampler, VK_RESOURCE_TYPE_SAMPLER);            
        } 

        Bindings[i] = {
            .binding = Safe_U32(i),
            .descriptorType = VK_Get_Descriptor_Type(CreateInfo.Bindings[i].Type),
            .descriptorCount = 1,
            .stageFlags = VK_Convert_To_Shader_Stage_Flags(CreateInfo.Bindings[i].StageFlags),
            .pImmutableSamplers = Samplers.Ptr
        };
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSeLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = Safe_U32(Bindings.Count),
        .pBindings = Bindings.Ptr
    };

    if(vkCreateDescriptorSetLayout(Context->Device, &DescriptorSeLayoutInfo, Context->VKAllocator, &Layout->Handle) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    return true;
}

internal void VK_Delete_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout) {
    if(Layout->Handle) {
        vkDestroyDescriptorSetLayout(Context->Device, Layout->Handle, Context->VKAllocator);
        Layout->Handle = VK_NULL_HANDLE;
    }
}

internal bool VK_Create_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup, const gdi_bind_group_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    
    vk_handle<vk_bind_group_layout> LayoutHandle(CreateInfo.Layout.ID);
    vk_bind_group_layout* Layout = VK_Resource_Get(ResourceContext->BindGroupLayouts, LayoutHandle);
    if(!Layout) {
        Assert(false);
        return false;
    }

    if(!VK_Descriptor_Pool_Allocate(&Context->DescriptorPool, Layout->Handle, &BindGroup->Handle)) {
        //todo: Diagnostics
        return false;
    }

    const gdi_bind_group_write_info* WriteInfo = &CreateInfo.WriteInfo;

    scratch Scratch = Scratch_Get();
    BindGroup->DynamicOffsets = array<uptr>(Context->GDI->MainAllocator);
    array<VkWriteDescriptorSet> DescriptorWrites(&Scratch, WriteInfo->Bindings.Count);
    array<VkCopyDescriptorSet> DescriptorCopies(&Scratch, WriteInfo->Bindings.Count);

    for(uptr i = 0; i < WriteInfo->Bindings.Count; i++) {
        VkDescriptorImageInfo* ImageInfo = NULL;
        VkDescriptorBufferInfo* BufferInfo = NULL;

        if(GDI_Is_Bind_Group_Buffer(WriteInfo->Bindings[i].Type)) {
            const gdi_bind_group_buffer* BufferBinding = &WriteInfo->Bindings[i].BufferBinding; 
            vk_handle<vk_buffer> BufferHandle(BufferBinding->Buffer.ID);
            vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
            if(Buffer) {
                BufferInfo = Scratch_Push_Struct(&Scratch, VkDescriptorBufferInfo);
                BufferInfo->buffer = Buffer->Handle;
                BufferInfo->offset = 0;
                BufferInfo->range  = BufferBinding->Size == (uptr)-1 ? VK_WHOLE_SIZE : BufferBinding->Size;

                if(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
                    Assert(GDI_Is_Bind_Group_Dynamic(WriteInfo->Bindings[i].Type));
                    Array_Push(&BindGroup->DynamicOffsets, Buffer->Size);
                }

                BindGroup->Add_Reference(Context, Buffer, VK_RESOURCE_TYPE_BUFFER);
            }
        } else if(GDI_Is_Bind_Group_Texture(WriteInfo->Bindings[i].Type)) {
            const gdi_bind_group_texture* TextureBinding = &WriteInfo->Bindings[i].TextureBinding;
            vk_handle<vk_texture_view> TextureViewHandle(TextureBinding->TextureView.ID);
            vk_texture_view* TextureView = VK_Resource_Get(ResourceContext->TextureViews, TextureViewHandle);
            if(TextureView) {
                ImageInfo = Scratch_Push_Struct(&Scratch, VkDescriptorImageInfo);
                ImageInfo->imageView = TextureView->Handle;
                ImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                BindGroup->Add_Reference(Context, TextureView, VK_RESOURCE_TYPE_TEXTURE_VIEW);
            }
        }

        if(ImageInfo || BufferInfo) {        
            Array_Push(&DescriptorWrites, {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = BindGroup->Handle,
                .dstBinding = Safe_U32(i),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_Get_Descriptor_Type(WriteInfo->Bindings[i].Type),
                .pImageInfo = ImageInfo,
                .pBufferInfo = BufferInfo
            });
        } else {
            const gdi_bind_group_copy* CopyBinding = &WriteInfo->Bindings[i].CopyBinding;
            vk_handle<vk_bind_group> Handle(CopyBinding->SrcBindGroup.ID);
            vk_bind_group* SrcBindGroup = VK_Resource_Get(ResourceContext->BindGroups, Handle);
            if(!SrcBindGroup) {
                Assert(false);
                return false;
            }

            Array_Push(&DescriptorCopies, {
                .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
                .srcSet = SrcBindGroup->Handle,
                .srcBinding = CopyBinding->SrcBindIndex,
                .dstSet = BindGroup->Handle,
                .dstBinding = Safe_U32(i),
                .descriptorCount = 1
            });

            BindGroup->Add_Reference(Context, SrcBindGroup->References[CopyBinding->SrcBindIndex]);
        }
    }
    
    //Add the layout as the final reference so we can easily reference binding group
    //bindings by index easily
    BindGroup->Add_Reference(Context, Layout, VK_RESOURCE_TYPE_BIND_GROUP_LAYOUT);
    vkUpdateDescriptorSets(Context->Device, Safe_U32(DescriptorWrites.Count), DescriptorWrites.Ptr, 
                           Safe_U32(DescriptorCopies.Count), DescriptorCopies.Ptr);

    return true;
}

internal void VK_Delete_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup) {
    Array_Free(&BindGroup->DynamicOffsets);
    if(BindGroup->Handle) {
        //vkAllocateDescriptorSets is not thread safe. Descriptor pool must be
        //externally synchronized
        VK_Descriptor_Pool_Free(&Context->DescriptorPool, BindGroup->Handle);
        BindGroup->Handle = VK_NULL_HANDLE;
    }
}

internal bool VK_Descriptor_Pool_Create(gdi_context* Context, vk_descriptor_pool* Pool) {
    VkDescriptorPoolSize PoolSizes[] = { {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 9999
        }, {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 9999
        }, {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 9999
        }, {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 9999
        }
    };
    static_assert(Array_Count(PoolSizes) == GDI_BIND_GROUP_TYPE_COUNT);
    
    VkDescriptorPoolCreateInfo PoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 9999,
        .poolSizeCount = Array_Count(PoolSizes),
        .pPoolSizes = PoolSizes
    };

    VkDescriptorPool DescriptorPool;
    if(vkCreateDescriptorPool(Context->Device, &PoolInfo, Context->VKAllocator, &DescriptorPool) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    Pool->Pool = DescriptorPool;
    Pool->Device = Context->Device;
    Pool->VKAllocator = Context->VKAllocator;
    AK_Mutex_Create(&Pool->Lock);
    return true;
}

internal void VK_Descriptor_Pool_Delete(vk_descriptor_pool* Pool) {
    AK_Mutex_Delete(&Pool->Lock);
    if(Pool->Pool) {
        vkDestroyDescriptorPool(Pool->Device, Pool->Pool, Pool->VKAllocator);
        Pool->Pool = VK_NULL_HANDLE;
        Pool->Device = VK_NULL_HANDLE;
        Pool->VKAllocator = VK_NULL_HANDLE;
    }
}

bool VK_Descriptor_Pool_Allocate(vk_descriptor_pool* Pool, VkDescriptorSetLayout SetLayout, VkDescriptorSet* OutDescriptorSet) {
    VkDescriptorSetAllocateInfo AllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = Pool->Pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &SetLayout
    };

    AK_Mutex_Lock(&Pool->Lock);
    if(vkAllocateDescriptorSets(Pool->Device, &AllocateInfo, OutDescriptorSet) != VK_SUCCESS) {
        AK_Mutex_Unlock(&Pool->Lock);
        //todo: Logging
        return false;
    }
    AK_Mutex_Unlock(&Pool->Lock);
    return true;
}

void VK_Descriptor_Pool_Free(vk_descriptor_pool* Pool, VkDescriptorSet DescriptorSet) {
    AK_Mutex_Lock(&Pool->Lock);
    vkFreeDescriptorSets(Pool->Device, Pool->Pool, 1, &DescriptorSet);
    AK_Mutex_Unlock(&Pool->Lock);
}