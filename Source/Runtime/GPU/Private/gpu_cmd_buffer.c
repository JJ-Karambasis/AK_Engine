void GPU_Cmd_Buffer_Reset(gpu_cmd_buffer* CmdBuffer) 
{
    Arena_Clear(CmdBuffer->CmdArena, MEMORY_NO_CLEAR);
    CmdBuffer->FirstCmd = CmdBuffer->LastCmd = NULL;
}

gpu_ui_pass* GPU_Cmd_Buffer_Begin_UI_Pass(gpu_cmd_buffer* CmdBuffer, gpu_ui_pass_begin_info* BeginInfo)
{
    gpu_cmd_begin_ui_pass* Cmd = Arena_Push_Struct(CmdBuffer->CmdArena, gpu_cmd_begin_ui_pass);
    Zero_Struct(Cmd, gpu_cmd_begin_ui_pass);
    Cmd->Cmd.Type = GPU_CMD_TYPE_BEGIN_UI_PASS;
    
    gpu_ui_pass* UIPass = &Cmd->UIPass;
    UIPass->CmdArena = Arena_Create(Get_Base_Allocator(CmdBuffer->CmdArena), Kilo(32));
    UIPass->ClearAttachments = BeginInfo->FramebufferInfo.Clear;
    UIPass->Framebuffer = BeginInfo->FramebufferInfo.Framebuffer;
    
    SLL_Push_Back(CmdBuffer->FirstCmd, CmdBuffer->LastCmd, &Cmd->Cmd);
    return UIPass;
}

void GPU_Cmd_Copy_Texture_To_Display(gpu_cmd_buffer* CmdBuffer, 
                                     gpu_display* Display, uint32_t DisplayOffsetX, uint32_t DisplayOffsetY, 
                                     gpu_texture2D* Texture, uint32_t TextureOffsetX, uint32_t TextureOffsetY, 
                                     uint32_t Width, uint32_t Height)
{
    gpu_cmd_copy_texture_to_display* Cmd = Arena_Push_Struct(CmdBuffer->CmdArena, gpu_cmd_copy_texture_to_display);
    Zero_Struct(Cmd, gpu_cmd_copy_texture_to_display);
    
    Assert(Display);
    Assert(Texture);
    
    Cmd->Cmd.Type   = GPU_CMD_TYPE_COPY_TEXTURE_TO_DISPLAY;
    Cmd->DstDisplay = Display;
    Cmd->DstOffsetX = DisplayOffsetX;
    Cmd->DstOffsetY = DisplayOffsetY;
    Cmd->SrcTexture = Texture;
    Cmd->SrcOffsetX = TextureOffsetX;
    Cmd->SrcOffsetY = TextureOffsetY;
    Cmd->Width      = Width;
    Cmd->Height     = Height;
    
    SLL_Push_Back(CmdBuffer->FirstCmd, CmdBuffer->LastCmd, &Cmd->Cmd);
}

void GPU_UI_Pass_Draw_Rectangle(gpu_ui_pass* UIPass, v2 Min, v2 Max, gpu_texture_unit TextureUnit, v4 Color)
{
    gpu_ui_pass_draw_rectangle* Cmd = Arena_Push_Struct(UIPass->CmdArena, gpu_ui_pass_draw_rectangle);
    Zero_Struct(Cmd, gpu_ui_pass_draw_rectangle);
    
    Cmd->Cmd.Type    = GPU_UI_PASS_CMD_TYPE_DRAW_RECTANGLE;
    Cmd->Min         = Min;
    Cmd->Max         = Max;
    Cmd->Color       = Color;
    Cmd->TextureUnit = TextureUnit;
    
    SLL_Push_Back(UIPass->FirstCmd, UIPass->LastCmd, &Cmd->Cmd);
}