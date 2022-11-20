#include <gl_shader_generated.c>
#include <common.shader>

global gpu_device_context_vtable G_DeviceContextVTable =
{
    GL_Device_Context_Get_Display_Manager,
    GL_Device_Context_Get_Resource_Manager,
    GL_Device_Context_Allocate_Cmd_Buffer,
    GL_Device_Context_Free_Cmd_Buffer,
    GL_Device_Context_Dispatch_Cmds
};

shared_export GPU_SET_DEVICE(GPU_Set_Device)
{
    gl* GL = (gl*)_Context;
    
    Assert(!GL->ContextManager->DeviceContext.Device);
    gl_device* Device = (gl_device*)_Device;
    if(!GL_Context_Manager_Set_Device_Context(GL->ContextManager, Device))
        return NULL;
    
    gl_device_context* DeviceContext = &GL->ContextManager->DeviceContext;
    DeviceContext->Device = Device;
    
    DeviceContext->Arena = Arena_Create(Get_Base_Allocator(GL->Arena), Kilo(16));
    Set_VTable(&DeviceContext->DeviceContext, &G_DeviceContextVTable);
    DeviceContext->ResourceManager = GL_Resource_Manager_Create(Get_Base_Allocator(DeviceContext->Arena), DeviceContext);
    DeviceContext->DisplayManager = GL_Display_Manager_Create(Get_Base_Allocator(DeviceContext->Arena), GL->ContextManager, DeviceContext->ResourceManager);
    DeviceContext->CmdPool = GL_Cmd_Pool_Create(Get_Base_Allocator(DeviceContext->Arena));
    
    gpu_sampler_create_info DefaultSamplerCreateInfo;
    Zero_Struct(&DefaultSamplerCreateInfo, gpu_sampler_create_info);
    DefaultSamplerCreateInfo.MinFilter = GPU_SAMPLER_FILTER_LINEAR;
    DefaultSamplerCreateInfo.MagFilter = GPU_SAMPLER_FILTER_LINEAR;
    DefaultSamplerCreateInfo.AddressModeU = GPU_SAMPLER_ADDRESS_MODE_CLAMP;
    DefaultSamplerCreateInfo.AddressModeV = GPU_SAMPLER_ADDRESS_MODE_CLAMP;
    DeviceContext->DefaultLinearSampler = (gl_sampler*)GL_Resource_Manager_Create_Sampler((gpu_resource_manager*)DeviceContext->ResourceManager, &DefaultSamplerCreateInfo);
    
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    const strc ShaderVersionCore = StrC_Lit("#version 460 core\n#define SHADER_CODE");
    for(uint32_t UIShaderIndex = 0; UIShaderIndex < Array_Count(DeviceContext->UIShaders.Shaders); UIShaderIndex++)
    {
        gl_device_ui_shader* UIShader = DeviceContext->UIShaders.Shaders + UIShaderIndex;
        gl_shader_builder* Shader = GL_Shader_Builder_Create(Scratch);
        
        GL_Shader_Builder_Add_Shader(Shader, ShaderVersionCore, GL_SHADER_TYPE_VERTEX);
        GL_Shader_Builder_Add_Shader(Shader, StrC_Lit("#define VERTEX_SHADER"), GL_SHADER_TYPE_VERTEX);
        
        GL_Shader_Builder_Add_Shader(Shader, ShaderVersionCore, GL_SHADER_TYPE_FRAGMENT);
        GL_Shader_Builder_Add_Shader(Shader, StrC_Lit("#define PIXEL_SHADER"), GL_SHADER_TYPE_FRAGMENT);
        
        switch(UIShaderIndex)
        {
            case UI_SHADER_TEXTURE_INDEX:
            {
                GL_Shader_Builder_Add_Shader(Shader, StrC_Lit("#define TEXTURED"), GL_SHADER_TYPE_VERTEX);
                GL_Shader_Builder_Add_Shader(Shader, StrC_Lit("#define TEXTURED"), GL_SHADER_TYPE_FRAGMENT);
            } break;
        }
        
        GL_Shader_Builder_Add_Shader(Shader, StrC_Lit(gl_rect_shader), GL_SHADER_TYPE_VERTEX);
        GL_Shader_Builder_Add_Shader(Shader, StrC_Lit(gl_rect_shader), GL_SHADER_TYPE_FRAGMENT);
        
        UIShader->Program = GL_Shader_Builder_Compile_Program(Shader);
        if(!UIShader->Program) 
        {
            strc ErrorString = GL_Shader_Builder_Get_Error_String(Shader, Scratch);
            Assert(false);
            return NULL;
        }
        
        UIShader->MinUniformID               = glGetUniformLocation(UIShader->Program, "Min");
        UIShader->MaxUniformID               = glGetUniformLocation(UIShader->Program, "Max");
        UIShader->InvHalfResolutionUniformID = glGetUniformLocation(UIShader->Program, "InvHalfResolution");
        UIShader->ColorUniformID             = glGetUniformLocation(UIShader->Program, "Color");   
        UIShader->TextureUniformID           = glGetUniformLocation(UIShader->Program, "Texture");
    }
    
    return &DeviceContext->DeviceContext;
}

GPU_GET_DISPLAY_MANAGER(GL_Device_Context_Get_Display_Manager)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    return &DeviceContext->DisplayManager->DisplayManager;
}

GPU_GET_RESOURCE_MANAGER(GL_Device_Context_Get_Resource_Manager)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    return &DeviceContext->ResourceManager->ResourceManager;
}

GPU_ALLOCATE_CMD_BUFFER(GL_Device_Context_Allocate_Cmd_Buffer)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    gl_cmd_buffer* CmdBuffer = GL_Cmd_Pool_Create_Buffer(DeviceContext->CmdPool);
    return &CmdBuffer->CmdBuffer;
}

GPU_FREE_CMD_BUFFER(GL_Device_Context_Free_Cmd_Buffer)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    gl_cmd_buffer* CmdBuffer = (gl_cmd_buffer*)_CmdBuffer;
    GL_Cmd_Pool_Delete_Buffer(DeviceContext->CmdPool, CmdBuffer);
}

void GL_Set_Framebuffer(gl_framebuffer* Framebuffer, gpu_clear_attachments* ClearAttachments)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->Handle);
    glViewport(0, 0, Framebuffer->Width, Framebuffer->Height);
    
    GLenum ClearBits = GL_NONE;
    if(ClearAttachments->ColorCount)
    {
        GLenum* DrawBuffers = Arena_Push_Array(Scratch, GLenum, ClearAttachments->ColorCount);
        for(uint32_t ColorIndex = 0; ColorIndex < ClearAttachments->ColorCount; ColorIndex++)
            DrawBuffers[ColorIndex] = GL_COLOR_ATTACHMENT0+ColorIndex;
        glDrawBuffers(ClearAttachments->ColorCount, DrawBuffers);
        
        for(uint32_t ColorIndex = 0; ColorIndex < ClearAttachments->ColorCount; ColorIndex++)
            glClearBufferfv(GL_COLOR, ColorIndex, ClearAttachments->ColorClears[ColorIndex].Float);
    }
    
    if(ClearAttachments->DepthStencilClear)
    {
        glClearBufferfv(GL_DEPTH, 0, &ClearAttachments->DepthStencilClear->Depth);
    }
}

GPU_DISPATCH_CMDS(GL_Device_Context_Dispatch_Cmds)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    arena* RenderPassStorage = Arena_Create(Get_Base_Allocator(Scratch), Mega(1));
    
    GL_Context_Make_Current(DeviceContext->Device->Context);
    
    gl_copy_texture_to_display_list CopyTextureToDisplayList;
    Zero_Struct(&CopyTextureToDisplayList, gl_copy_texture_to_display_list);
    
    gl_ui_render_pass_list UIRenderPasses;
    Zero_Struct(&UIRenderPasses, gl_ui_render_pass_list);
    
    for(uint32_t CmdBufferIndex = 0; CmdBufferIndex < Count; CmdBufferIndex++)
    {
        const gpu_cmd_buffer* CmdBuffer = CmdBuffers[CmdBufferIndex];
        for(gpu_cmd* BaseCmd = CmdBuffer->FirstCmd; BaseCmd; BaseCmd = BaseCmd->Next)
        {
            switch(BaseCmd->Type)
            {
                case GPU_CMD_TYPE_BEGIN_UI_PASS:
                {
                    gpu_cmd_begin_ui_pass* Cmd = (gpu_cmd_begin_ui_pass*)BaseCmd;
                    gpu_ui_pass* UIPass = &Cmd->UIPass;
                    
                    gl_ui_render_pass* RenderPass = Arena_Push_Struct(RenderPassStorage, gl_ui_render_pass);
                    SLL_Push_Back(UIRenderPasses.First, UIRenderPasses.Last, RenderPass);
                    UIRenderPasses.Count++;
                    
                    RenderPass->Framebuffer = (gl_framebuffer*)UIPass->Framebuffer;
                    RenderPass->ClearAttachments = UIPass->ClearAttachments;
                    
                    for(gpu_ui_pass_cmd* BaseUICmd = UIPass->FirstCmd; BaseUICmd; BaseUICmd = BaseUICmd->Next)
                    {
                        switch(BaseUICmd->Type)
                        {
                            case GPU_UI_PASS_CMD_TYPE_DRAW_RECTANGLE:
                            {
                                gpu_ui_pass_draw_rectangle* UICmd = (gpu_ui_pass_draw_rectangle*)BaseUICmd;
                                
                                //TODO(JJ): Opacity
                                gl_ui_rectangle_draw* Draw = Arena_Push_Struct(RenderPassStorage, gl_ui_rectangle_draw);
                                Draw->Min   = UICmd->Min;
                                Draw->Max   = UICmd->Max;
                                Draw->Color = UICmd->Color;
                                SLL_Push_Back(RenderPass->ColorDraws.First, RenderPass->ColorDraws.Last, Draw);
                                RenderPass->ColorDraws.Count++;
                            } break;
                        }
                    }
                } break;
                
                case GPU_CMD_TYPE_UPLOAD_TEXTURE:
                {
                    gl_cmd_upload_texture* Cmd = (gl_cmd_upload_texture*)BaseCmd;
                    
                    gl_texture2D* Texture = Cmd->DstTexture;
                    gpu_texture_format Format = Texture->Format;
                    
                    glBindTexture(GL_TEXTURE_2D, Cmd->DstTexture->Handle);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, Cmd->DstOffsetX, Cmd->DstOffsetY, Cmd->SrcWidth, Cmd->SrcHeight, G_FormatMap[Format], G_TypeMap[Format], Cmd->SrcTexels);
                    glBindTexture(GL_TEXTURE_2D, 0);
                } break;
                
                case GPU_CMD_TYPE_COPY_TEXTURE_TO_DISPLAY:
                {
                    const gpu_cmd_copy_texture_to_display* Cmd = (const gpu_cmd_copy_texture_to_display*)BaseCmd;
                    
                    gl_copy_texture_to_display* Copy = Arena_Push_Struct(RenderPassStorage, gl_copy_texture_to_display);
                    Copy->DstDisplay = (gl_display*)Cmd->DstDisplay;
                    Copy->DstOffsetX = Cmd->DstOffsetX;
                    Copy->DstOffsetY = Cmd->DstOffsetY;
                    Copy->SrcTexture = (gl_texture2D*)Cmd->SrcTexture;
                    Copy->SrcOffsetX = Cmd->SrcOffsetX;
                    Copy->SrcOffsetY = Cmd->SrcOffsetY;
                    Copy->Width      = Cmd->Width;
                    Copy->Height     = Cmd->Height;
                    
                    SLL_Push_Back(CopyTextureToDisplayList.First, CopyTextureToDisplayList.Last, Copy);
                    CopyTextureToDisplayList.Count++;
                } break;
            }
        }  
    }
    
    for(gl_ui_render_pass* RenderPass = UIRenderPasses.First; RenderPass; RenderPass = RenderPass->Next)
    {
        GL_Set_Framebuffer(RenderPass->Framebuffer, &RenderPass->ClearAttachments);
        
        if(RenderPass->ColorDraws.Count)
        {
            gl_device_ui_shader* Shader = &DeviceContext->UIShaders.Shaders[0];
            
            glUseProgram(Shader->Program);
            glBindVertexArray(DeviceContext->Device->Context->EmptyVAO);
            
            v2 HalfResolution = V2_Mul_S(V2((float)RenderPass->Framebuffer->Width, 
                                            (float)RenderPass->Framebuffer->Height), 0.5f);
            v2 InvHalfResolution = V2_Inv(HalfResolution);
            
            glUniform2f(Shader->InvHalfResolutionUniformID, InvHalfResolution.x, InvHalfResolution.y);
            
            for(gl_ui_rectangle_draw* Draw = RenderPass->ColorDraws.First; Draw; Draw = Draw->Next)
            {
                uint32_t Color = Color_V4_To_U32(Draw->Color);
                
                glUniform2f(Shader->MinUniformID, Draw->Min.x, Draw->Min.y);
                glUniform2f(Shader->MaxUniformID, Draw->Max.x, Draw->Max.y);
                glUniform1ui(Shader->ColorUniformID, Color);
                
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    for(gl_copy_texture_to_display* Copy = CopyTextureToDisplayList.First; Copy; Copy = Copy->Next)
    {
        gl_display* Display = Copy->DstDisplay;
        gl_texture2D* Texture = Copy->SrcTexture;
        GL_Context_Make_Current(Display->Context);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        gl_device_ui_shader* Shader = &DeviceContext->UIShaders.Shaders[1];
        
        glViewport(Copy->DstOffsetX, Copy->DstOffsetY, Copy->Width, Copy->Height);
        
        glUseProgram(Shader->Program);
        glBindVertexArray(Display->Context->EmptyVAO);
        
        v2 HalfResolution = V2_Mul_S(V2((float)Display->Width, (float)Display->Height), 0.5f);
        v2 InvHalfResolution = V2_Inv(HalfResolution);
        
        glUniform2f(Shader->InvHalfResolutionUniformID, InvHalfResolution.x, InvHalfResolution.y);
        glUniform2f(Shader->MinUniformID, (float)Copy->SrcOffsetX, (float)Copy->SrcOffsetY);
        glUniform2f(Shader->MaxUniformID, (float)(Copy->SrcOffsetX+Copy->Width), (float)(Copy->SrcOffsetY+Copy->Height));
        glUniform1i(Shader->TextureUniformID, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture->Handle);
        glBindSampler(0, DeviceContext->DefaultLinearSampler->Handle);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        GL_Context_Make_Current(DeviceContext->Device->Context);
    }
}