local_persist DRAW_CALLBACK(UI_Renderer_Update) {
    ui_renderer* Renderer = (ui_renderer*)UserData;
    scratch Scratch = Scratch_Get();
    ui* UI = Renderer->UI;
    array<ui_box_shader_box> DrawBoxes(&Scratch);
    
    array<ui_box*> BoxStack(&Scratch, UI->BoxHashTable.Count);
    if(UI->Root) {
        Array_Push(&BoxStack, UI->Root);
        while(!Array_Empty(&BoxStack)) {
            ui_box* Box = Array_Pop(&BoxStack);

            if(!Rect2_Is_Empty(Box->Rect)) {
                Array_Push(&DrawBoxes, {
                    .DstP0 = Box->Rect.Min,
                    .DstP1 = Box->Rect.Max,
                    .Color = Box->BackgroundColor
                });
            }

            for(ui_box* Child = Box->FirstChild; Child; Child = Child->NextSibling) {
                Array_Push(&BoxStack, Child);
            }
        }
    }

    
#if 0 
    string ArabTextString = String_Lit("T.W.Lewis");
    uba* UBA = UBA_Allocate(&Scratch, ArabTextString);
    span<uba_run> Runs = UBA_Get_Runs(UBA);
    span<uba_script_info> ScriptInfo = UBA_Get_Scripts(UBA);

    uptr RunIndex = 0;
    uptr ScriptIndex = 0;
    const uba_run* CurrentRun = &Runs[RunIndex++];
    const uba_script_info* CurrentScript = &ScriptInfo[ScriptIndex++];

    vec2 Cursor = vec2(100, 100);


    uptr StartIndex = 0;
    uptr CurrentIndex = 0;
    for(; CurrentIndex <= ArabTextString.Size; CurrentIndex++) {
        bool RunInRange = In_Range(CurrentIndex, CurrentRun->Offset, (CurrentRun->Offset+CurrentRun->Length)-1);
        bool ScriptInRange = In_Range(CurrentIndex, CurrentScript->Offset, (CurrentScript->Offset+CurrentScript->Length)-1);

        if(!RunInRange || !ScriptInRange) {
            string StringRange = String_Substr(ArabTextString, StartIndex, CurrentIndex);
            StartIndex = CurrentIndex;

            //Remove the font manager. We will have another way of detecting
            //what font to use in more arbitrary cases later
            font_id Font = UI->FontID;
            u32 Ascender = Font_Get_Metrics(Font)->Ascender;
            
            text_shape_result ShapeResult = Font_Shape(Font, {
                .Allocator = &Scratch,
                .Text = StringRange,
                .Properties = {
                    .Direction = CurrentRun->Direction,
                    .Script = CurrentScript->Script
                }
            });

            const text_shape_pos* Positions = ShapeResult.Positions;
            const text_glyph_info* Glyphs = ShapeResult.Glyphs;
            for(u32 i = 0; i < ShapeResult.GlyphCount; i++) {
                const glyph_entry* Glyph = Glyph_Cache_Get(UI->GlyphCache, Font, Glyphs[i].Codepoint);
                glyph_metrics Metrics = Font_Get_Glyph_Metrics(Font, Glyphs[i].Codepoint);
                
                if(i != 0) {
                    svec2 Kerning = Font_Get_Kerning(Font, Glyphs[i-1].Codepoint, Glyphs[i].Codepoint);
                    Cursor.x += (f32)Kerning.x;
                }

                if(Glyph) {
                    //todo: Not sure if our offsets are properly calculated when the
                    //text shaper offsets are not zero
                    vec2 Offset = vec2(Positions[i].Offset + Metrics.Offset);
                    Offset.y = (f32)Ascender - Offset.y;
                    vec2 PixelP = Cursor + Offset;

                    Array_Push(&Boxes, {
                        .DstP0 = PixelP,
                        .DstP1 = PixelP + Rect2u_Get_Dim(Glyph->AtlasRect),
                        .SrcP0 = Glyph->AtlasRect.Min,
                        .SrcP1 = Glyph->AtlasRect.Max,
                        .Color = vec4(1.0f, 1.0f, 1.0f, 1.0f)
                    });
                }

                Cursor += vec2(Positions[i].Advance);
            }

            if(!RunInRange) {
                CurrentRun = Span_Get(&Runs, RunIndex++);
            }

            if(!ScriptInRange) {
                CurrentScript = Span_Get(&ScriptInfo, ScriptIndex++);
            }
        }
    }
#endif

    //If we don't have any boxes to render we can just early skip
    if(!DrawBoxes.Count) return;
    
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

    ui_box_shader_global ShaderGlobal = {
        .InvRes = 1.0f / vec2(Resolution),
        .InvTexRes = 1.0f / vec2(Renderer->GlyphCache->Atlas.Dim)
    };

    GDI_Context_Buffer_Write(Context, Renderer->GlobalBuffer, const_buffer(&ShaderGlobal));
    GDI_Context_Buffer_Write(Context, Renderer->InstanceBuffer, const_buffer(span(DrawBoxes)));

    Draw_Stream_Set_Pipeline(DrawStream, Renderer->Pipeline.Pipeline);
    Draw_Stream_Set_Bind_Groups(DrawStream, {Renderer->GlobalBindGroup});
    Draw_Stream_Set_Vtx_Buffers(DrawStream, {Renderer->InstanceBuffer});
    Draw_Stream_Draw_Vtx(DrawStream, 0, 0, 4, Safe_U32(DrawBoxes.Count));
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
    gdi_handle<gdi_sampler> Sampler = GDI_Context_Create_Sampler(Context, {
        .Filter = GDI_FILTER_LINEAR
    });

    gdi_handle<gdi_bind_group_layout> BindGroupLayout = GDI_Context_Create_Bind_Group_Layout(Context, {
        .Bindings = { { 
                .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                .StageFlags = GDI_SHADER_STAGE_VERTEX_BIT
            }, {
                .Type = GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
                 .StageFlags = GDI_SHADER_STAGE_PIXEL_BIT
            }, {
                .Type = GDI_BIND_GROUP_TYPE_SAMPLER,
                .StageFlags = GDI_SHADER_STAGE_PIXEL_BIT,
                .ImmutableSamplers = {Sampler}
            }
        }
    });

    scratch Scratch = Scratch_Get();
    package* UIBoxShaderPackage = Packages_Get_Package(Packages, String_Lit("shaders"), String_Lit("ui_box")); 
    resource* UIBoxVtxShaderResource = Packages_Get_Resource(Packages, UIBoxShaderPackage, String_Lit("vtx_shader"));
    resource* UIBoxPxlShaderResource = Packages_Get_Resource(Packages, UIBoxShaderPackage, String_Lit("pxl_shader"));

    const_buffer VtxShader = Packages_Load_Entire_Resource(Packages, UIBoxVtxShaderResource, &Scratch);
    const_buffer PxlShader = Packages_Load_Entire_Resource(Packages, UIBoxPxlShaderResource, &Scratch);

    gdi_handle<gdi_pipeline> Pipeline = GDI_Context_Create_Graphics_Pipeline(Context, {
        .VS = {VtxShader, String_Lit("VS_Main")},
        .PS = {PxlShader, String_Lit("PS_Main")},
        .Layouts = {BindGroupLayout},
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
        .Sampler = Sampler,
        .BindGroupLayout = BindGroupLayout,
        .Pipeline = Pipeline
    };
}

void UI_Renderer_Create(ui_renderer* UIRenderer, renderer* Renderer, ui_render_pass* RenderPass, ui_pipeline* Pipeline, ui* UI, glyph_cache* GlyphCache) {    
    UIRenderer->Pipeline = *Pipeline;
    UIRenderer->UI = UI;
    UIRenderer->GlyphCache = GlyphCache;

    gdi_context* Context = Renderer_Get_Context(Renderer);

    UIRenderer->GlobalBuffer = GDI_Context_Create_Buffer(Context, {
        .ByteSize = sizeof(ui_box_shader_global),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT
    });
    if(UIRenderer->GlobalBuffer.Is_Null()) {
        Assert(false);
        return;
    }

    UIRenderer->GlobalBindGroup = GDI_Context_Create_Bind_Group(Context, {
        .Layout = UIRenderer->Pipeline.BindGroupLayout,
        .WriteInfo = { 
            .Bindings = { { 
                    .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                    .BufferBinding = {
                        .Buffer = UIRenderer->GlobalBuffer
                    }
                }, {
                    .Type = GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
                    .CopyBinding = {
                        .SrcBindGroup = UIRenderer->GlyphCache->Atlas.BindGroup
                    }
                }
            }
        }
    });

    UIRenderer->RenderTask = Renderer_Create_Draw_Task(Renderer, UI_Renderer_Update, UIRenderer);
}