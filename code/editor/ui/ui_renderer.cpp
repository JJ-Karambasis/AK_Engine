internal DRAW_CALLBACK(UI_Renderer_Update) {
    ui* UI = (ui*)UserData;
    ui_renderer* Renderer = &UI->Renderer;
    scratch Scratch = Scratch_Get();

    array<ui_box_instance>  BoxInstances(&Scratch);
    array<ui_box_shader_box> DrawBoxes(&Scratch, UI->RenderBoxCount);
    
    gdi_handle<gdi_bind_group> CurrentTexture;
    u32 InstanceCount = 0;

    for(ui_render_box_entry* RenderBox = UI->FirstRenderBox; RenderBox; RenderBox = RenderBox->Next) {
        if(CurrentTexture != RenderBox->Texture) {
            if(BoxInstances.Count) {
                ui_box_instance* Instance = &Array_Last(&BoxInstances);
                Instance->InstanceCount = InstanceCount;
                InstanceCount = 0;
            }   

            CurrentTexture = RenderBox->Texture; 
            Array_Push(&BoxInstances, {
                .BindGroup = CurrentTexture,
                .Dim = RenderBox->TextureDim
            });
        }

        Array_Push(&DrawBoxes, {
            .DstP0 = RenderBox->ScreenRect.Min,
            .DstP1 = RenderBox->ScreenRect.Max,
            .SrcP0 = RenderBox->UVRect.Min,
            .SrcP1 = RenderBox->UVRect.Max,
            .Color = RenderBox->Color
        });

        InstanceCount++;
    }

    if(InstanceCount > 0) {
        Assert(BoxInstances.Count > 0);
        ui_box_instance* Instance = &Array_Last(&BoxInstances);
        Instance->InstanceCount = InstanceCount;
        InstanceCount = 0;
    }

    //If we don't have any boxes to render we can just early skip
    if(!DrawBoxes.Count || !BoxInstances.Count) return;
    
    if(DrawBoxes.Count > Renderer->InstanceCount) {
        if(!Renderer->InstanceBuffer.Is_Null()) {
            GDI_Context_Delete_Buffer(Context, Renderer->InstanceBuffer);
            Renderer->InstanceBuffer = {};
        }

        Renderer->InstanceCount = DrawBoxes.Count;
        Renderer->InstanceBuffer = GDI_Context_Create_Buffer(Context, {
            .ByteSize   = Renderer->InstanceCount*sizeof(ui_box_shader_box),
            .UsageFlags = GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_GPU_LOCAL_BUFFER_BIT 
        });
    }
    GDI_Context_Buffer_Write(Context, Renderer->InstanceBuffer, const_buffer(span(DrawBoxes)));

    Draw_Stream_Set_Pipeline(DrawStream, Renderer->Pipeline.Pipeline);
    Draw_Stream_Set_Vtx_Buffers(DrawStream, {Renderer->InstanceBuffer});

    u32 InstanceOffset = 0;
    for(ui_box_instance Instance : BoxInstances) {
        dynamic_binding InstanceData = Dynamic_Buffer_Push<ui_box_shader_info>(DynamicBuffer);
        ui_box_shader_info* ShaderInfo = (ui_box_shader_info*)InstanceData.Data;
        ShaderInfo->InvRes = 1.0f / vec2(Resolution);
        ShaderInfo->InvTexRes = 1.0f / vec2(Instance.Dim);
        
        Draw_Stream_Set_Bind_Groups(DrawStream, {Instance.BindGroup});
        Draw_Stream_Set_Dynamic_Bind_Groups(DrawStream, {InstanceData.BindGroup}, {InstanceData.Offset});

        Draw_Stream_Draw_Vtx(DrawStream, 0, InstanceOffset, 4, Instance.InstanceCount);

        InstanceOffset += Instance.InstanceCount;
    }
    Assert(InstanceOffset == DrawBoxes.Count);
}

ui_render_pass UI_Render_Pass_Create(renderer* Renderer, gdi_format Format) {
    gdi_context* Context = Renderer_Get_Context(Renderer);
    ui_render_pass Result = {
        .RenderPass = GDI_Context_Create_Render_Pass(Context, {
            .Attachments = {
                gdi_render_pass_attachment::Color(Format, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
            }
        })
    };
    return Result;
}

ui_pipeline UI_Pipeline_Create(renderer* Renderer, packages* Packages, ui_render_pass* RenderPass) {
    gdi_context* Context = Renderer_Get_Context(Renderer);
    gdi_handle<gdi_bind_group_layout> TextureLayout = Renderer_Get_Texture_Layout(Renderer);
    gdi_handle<gdi_bind_group_layout> DynamicLayout = Renderer_Get_Dynamic_Layout(Renderer);

    scratch Scratch = Scratch_Get();
    package* UIBoxShaderPackage = Packages_Get_Package(Packages, String_Lit("shaders"), String_Lit("ui_box")); 
    resource* UIBoxVtxShaderResource = Packages_Get_Resource(Packages, UIBoxShaderPackage, String_Lit("vtx_shader"));
    resource* UIBoxPxlShaderResource = Packages_Get_Resource(Packages, UIBoxShaderPackage, String_Lit("pxl_shader"));

    const_buffer VtxShader = Packages_Load_Entire_Resource(Packages, UIBoxVtxShaderResource, &Scratch);
    const_buffer PxlShader = Packages_Load_Entire_Resource(Packages, UIBoxPxlShaderResource, &Scratch);

    gdi_handle<gdi_pipeline> Pipeline = GDI_Context_Create_Graphics_Pipeline(Context, {
        .VS = {VtxShader, String_Lit("VS_Main")},
        .PS = {PxlShader, String_Lit("PS_Main")},
        .Layouts = {TextureLayout, DynamicLayout},
        .GraphicsState = {
            .VtxBufferBindings = { {
                    .ByteStride = sizeof(ui_box_shader_box),
                    .InputRate = GDI_VTX_INPUT_RATE_INSTANCE,
                    .Attributes = { { 
                            .Semantic = String_Lit("Position"),
                            .SemanticIndex = 0,
                            .ByteOffset = offsetof(ui_box_shader_box, DstP0),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Position"),
                            .SemanticIndex = 1,
                            .ByteOffset = offsetof(ui_box_shader_box, DstP1),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Position"),
                            .SemanticIndex = 2,
                            .ByteOffset = offsetof(ui_box_shader_box, SrcP0),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Position"),
                            .SemanticIndex = 3,
                            .ByteOffset = offsetof(ui_box_shader_box, SrcP1),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Color"),
                            .SemanticIndex = 0,
                            .ByteOffset = offsetof(ui_box_shader_box, Color),
                            .Format = GDI_FORMAT_R32G32B32A32_FLOAT
                        }
                    }
                }
            },
            .Topology = GDI_TOPOLOGY_TRIANGLE_STRIP,
            .BlendStates = { { 
                    .BlendEnabled = true,
                    .SrcColor = GDI_BLEND_ONE,
                    .DstColor = GDI_BLEND_ONE_MINUS_SRC_ALPHA
                }
            }
        },
        .RenderPass = RenderPass->RenderPass
    });

    return {
        .Pipeline = Pipeline
    };
}

void UI_Renderer_Create(ui_renderer* UIRenderer, renderer* Renderer, ui_pipeline* Pipeline, ui* UI) {    
    UIRenderer->Pipeline = *Pipeline;
    UIRenderer->RenderTask = Renderer_Create_Draw_Task(Renderer, UI_Renderer_Update, UI);
}