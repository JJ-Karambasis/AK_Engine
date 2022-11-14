gl_cmd_pool* GL_Cmd_Pool_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(16));
    gl_cmd_pool* Result = Arena_Push_Struct(Arena, gl_cmd_pool);
    Zero_Struct(Result, gl_cmd_pool);
    Result->PoolMemory = Arena;
    return Result;
}

void GL_Cmd_Pool_Delete(gl_cmd_pool* CmdPool)
{
    Arena_Delete(CmdPool->PoolMemory);
}

static gpu_cmd_buffer_vtable G_GLCmdBufferVTable = 
{
    GL_Cmd_Buffer_Upload_Texture
};

gl_cmd_buffer* GL_Cmd_Pool_Create_Buffer(gl_cmd_pool* CmdPool)
{
    gl_cmd_storage* Storage = CmdPool->FreeStorage;
    if(!Storage) 
    {
        Storage = Arena_Push_Struct(CmdPool->PoolMemory, gl_cmd_storage);
        Storage->Arena = Arena_Create(Get_Base_Allocator(CmdPool->PoolMemory), Mega(4));
        Storage->Next = NULL;
    }
    else
    {
        Arena_Clear(Storage->Arena, MEMORY_NO_CLEAR);
        SLL_Pop_Front(CmdPool->FreeStorage);
    }
    
    gl_cmd_buffer* Result = Arena_Push_Struct(Storage->Arena, gl_cmd_buffer);
    Zero_Struct(Result, gl_cmd_buffer);
    Result->CmdBuffer.CmdArena = Arena_Create(Get_Base_Allocator(Storage->Arena), Mega(3)+Kilo(999));
    Set_VTable(&Result->CmdBuffer, &G_GLCmdBufferVTable);
    Result->CmdStorage = Storage;
    return Result;
}

void GL_Cmd_Pool_Delete_Buffer(gl_cmd_pool* CmdPool, gl_cmd_buffer* CmdBuffer)
{
    SLL_Push_Front(CmdPool->FreeStorage, CmdBuffer->CmdStorage);
    CmdBuffer->CmdStorage = NULL;
}

GPU_CMD_UPLOAD_TEXTURE_PROC(GL_Cmd_Buffer_Upload_Texture)
{
    gl_cmd_buffer* CmdBuffer = (gl_cmd_buffer*)_CmdBuffer;
    gl_cmd_upload_texture* Cmd = Arena_Push_Struct(CmdBuffer->CmdBuffer.CmdArena, gl_cmd_upload_texture);
    Zero_Struct(Cmd, gl_cmd_upload_texture);
    Cmd->Cmd.Type = GPU_CMD_TYPE_UPLOAD_TEXTURE;
    Cmd->DstTexture = (gl_texture2D*)_DstTexture;
    Cmd->DstOffsetX = DstOffsetX;
    Cmd->DstOffsetY = DstOffsetY;
    
    uint32_t BytesPerTexel = G_BytesPerPixel[Cmd->DstTexture->Format];
    size_t DataSize = BytesPerTexel*SrcWidth*SrcHeight;
    
    Cmd->SrcTexels = Arena_Push(CmdBuffer->CmdBuffer.CmdArena, DataSize, MEMORY_NO_CLEAR);
    Memory_Copy(Cmd->SrcTexels, SrcTexels, DataSize);
    Cmd->SrcWidth = SrcWidth;
    Cmd->SrcHeight = SrcHeight;
    
    SLL_Push_Back(CmdBuffer->CmdBuffer.FirstCmd, CmdBuffer->CmdBuffer.LastCmd, &Cmd->Cmd);
}