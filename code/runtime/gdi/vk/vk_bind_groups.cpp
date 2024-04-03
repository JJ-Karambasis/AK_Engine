internal bool VK_Create_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout, const gdi_bind_group_layout_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get(); 
    fixed_array<VkDescriptorSetLayoutBinding> Bindings(&Scratch, CreateInfo.Bindings.Count);
    
    for(uptr i = 0; i < CreateInfo.Bindings.Count; i++) {
        Bindings[i] = {
            .binding = Safe_U32(i),
            .descriptorType = VK_Get_Descriptor_Type(CreateInfo.Bindings[i].Type),
            .descriptorCount = 1,
            .stageFlags = VK_Convert_To_Shader_Stage_Flags(CreateInfo.Bindings[i].StageFlags)
        };
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSeLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = Safe_U32(Bindings.Count),
        .pBindings = Bindings.Ptr
    };

    if(vkCreateDescriptorSetLayout(Context->Device, &DescriptorSeLayoutInfo, Context->VKAllocator, &Layout->SetLayout) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    return true;
}

internal void VK_Delete_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout) {
    if(Layout->SetLayout) {
        vkDestroyDescriptorSetLayout(Context->Device, Layout->SetLayout, Context->VKAllocator);
        Layout->SetLayout = VK_NULL_HANDLE;
    }
}

internal void VK_Bind_Group_Layout_Record_Frame(gdi_context* Context, async_handle<vk_bind_group_layout> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.BindGroupLayoutsInUse[Handle.Index()], true);
}

internal bool VK_Create_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup, gdi_handle<gdi_bind_group_layout> Handle) {
    async_handle<vk_bind_group_layout> LayoutHandle(Handle.ID);
    vk_bind_group_layout* Layout = Async_Pool_Get(&Context->ResourceContext.BindGroupLayouts, LayoutHandle);
    if(!Layout) {
        Assert(false);
        return false;
    }

    if(!VK_Descriptor_Pool_Allocate(&Context->DescriptorPool, Layout->SetLayout, &BindGroup->DescriptorSet)) {
        //todo: Diagnostics
        return false;
    }

    BindGroup->DynamicOffsets = array<uptr>(Context->GDI->MainAllocator);
    return true;
}

internal void VK_Delete_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup) {
    Array_Free(&BindGroup->DynamicOffsets);
    if(BindGroup->DescriptorSet) {
        //vkAllocateDescriptorSets is not thread safe. Descriptor pool must be
        //externally synchronized
        VK_Descriptor_Pool_Free(&Context->DescriptorPool, BindGroup->DescriptorSet);
        BindGroup->DescriptorSet = VK_NULL_HANDLE;
    }
}

internal bool VK_Bind_Group_Write(gdi_context* Context, vk_bind_group* BindGroup, const gdi_bind_group_write_info& WriteInfo) {
    scratch Scratch = Scratch_Get();
    fixed_array<VkWriteDescriptorSet> DescriptorWrites(&Scratch, WriteInfo.Bindings.Count);

    for(uptr i = 0; i < WriteInfo.Bindings.Count; i++) {
        VkDescriptorImageInfo* ImageInfo = NULL;
        VkDescriptorBufferInfo* BufferInfo = NULL;

        if(GDI_Is_Bind_Group_Buffer(WriteInfo.Bindings[i].Type)) {
            BufferInfo = Scratch_Push_Struct(&Scratch, VkDescriptorBufferInfo);
            async_handle<vk_buffer> BufferHandle(WriteInfo.Bindings[i].BufferBinding.Buffer.ID);
            vk_buffer* Buffer = Async_Pool_Get(&Context->ResourceContext.Buffers, BufferHandle);
            if(!Buffer) {
                Assert(false);
                return false;
            }

            BufferInfo->buffer = Buffer->Buffer;
            BufferInfo->offset = 0;
            BufferInfo->range = VK_WHOLE_SIZE;

            if(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
                Assert(GDI_Is_Bind_Group_Dynamic(WriteInfo.Bindings[i].Type));
                Array_Push(&BindGroup->DynamicOffsets, Buffer->Size);
                BufferInfo->range = Buffer->Size;
            }
        } else {
            Assert(false);
        }
        
        DescriptorWrites[i] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = BindGroup->DescriptorSet,
            .dstBinding = Safe_U32(i),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_Get_Descriptor_Type(WriteInfo.Bindings[i].Type),
            .pImageInfo = ImageInfo,
            .pBufferInfo = BufferInfo
        };
    }
    
    vkUpdateDescriptorSets(Context->Device, Safe_U32(DescriptorWrites.Count), DescriptorWrites.Ptr, 0, VK_NULL_HANDLE);
    return true;
}

internal void VK_Bind_Group_Record_Frame(gdi_context* Context, async_handle<vk_bind_group> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.BindGroupsInUse[Handle.Index()], true);
}

internal bool VK_Descriptor_Pool_Create(gdi_context* Context, vk_descriptor_pool* Pool) {
    VkDescriptorPoolSize PoolSizes[] = { {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 9999
        }, {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
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