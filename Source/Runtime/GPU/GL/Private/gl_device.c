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

GPU_DISPATCH_CMDS(GL_Device_Context_Dispatch_Cmds)
{
    gl_device_context* DeviceContext = (gl_device_context*)_DeviceContext;
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    GL_Context_Make_Current(DeviceContext->Device->Context);
    
    gl_display* Display = NULL;
    for(uint32_t CmdBufferIndex = 0; CmdBufferIndex < Count; CmdBufferIndex++)
    {
        const gpu_cmd_buffer* CmdBuffer = CmdBuffers[CmdBufferIndex];
        for(gpu_cmd* BaseCmd = CmdBuffer->FirstCmd; BaseCmd; BaseCmd = BaseCmd->Next)
        {
            switch(BaseCmd->Type)
            {
                case GPU_CMD_TYPE_COPY_TEXTURE_TO_DISPLAY:
                {
                    const gpu_cmd_copy_texture_to_display* Cmd = (const gpu_cmd_copy_texture_to_display*)BaseCmd;
                    Display = (gl_display*)Cmd->DstDisplay;
                } break;
            }
        }  
    }
    
    local GLuint DEBUGShaderProgram;
    if(!DEBUGShaderProgram)
    {
        strc VertexShader
            = StrC_Null_Term("#version 460 core\n"
                             "int QuadIndices[6] = {0, 1, 2, 2, 3, 0};\n"
                             "vec2 QuadVertices[4] = {vec2(-0.5f, -0.5f), vec2( 0.5f, -0.5f), vec2( 0.5f, 0.5f), vec2(-0.5f, 0.5f)};\n"
                             "void main()\n"
                             "{\n"
                             "gl_Position = vec4(QuadVertices[QuadIndices[gl_VertexID]], 0.0f, 1.0f);\n"
                             "}\n");
        
        strc FragmentShader 
            = StrC_Null_Term("#version 460 core\n"
                             "layout(location = 0) out vec4 Color;\n"
                             "void main()\n"
                             "{\n"
                             "Color = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
                             "}\n");
        
        gl_shader_builder* Shader = GL_Shader_Builder_Create(Scratch);
        GL_Shader_Builder_Add_Shader(Shader, VertexShader, GL_SHADER_TYPE_VERTEX);
        GL_Shader_Builder_Add_Shader(Shader, FragmentShader, GL_SHADER_TYPE_FRAGMENT);
        DEBUGShaderProgram = GL_Shader_Builder_Compile_Program(Shader);
        if(!DEBUGShaderProgram)
        {
            strc ErrorString = GL_Shader_Builder_Get_Error_String(Shader, Scratch);
            OS_Debug_Log(Ascii_To_UTF8(Get_Base_Allocator(Scratch), ErrorString));
            Assert(false);
        }
    }
    
    GL_Context_Make_Current(Display->Context);
    
    glViewport(0, 0, 1920, 1080);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(DEBUGShaderProgram);
    glBindVertexArray(Display->Context->EmptyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}