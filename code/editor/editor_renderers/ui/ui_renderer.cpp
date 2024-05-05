ui_render_pass UI_Render_Pass_Create(gdi_context* Context, gdi_format Format) {
    ui_render_pass Result = {
        .RenderPass = GDI_Context_Create_Render_Pass(Context, {
            .Attachments = {
                gdi_render_pass_attachment::Color(Format, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
            }
        })
    };
    return Result;
}

ui_pipeline UI_Pipeline_Create(gdi_context* Context, packages* Packages, ui_render_pass* RenderPass) {
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

void UI_Renderer_Create(ui_renderer* Renderer, gdi_context* Context, ui_render_pass* RenderPass, ui_pipeline* Pipeline, ui* UI, glyph_cache* GlyphCache) {    
    Renderer->Context = Context;
    Renderer->RenderPass = *RenderPass;
    Renderer->Pipeline = *Pipeline;
    Renderer->UI = UI;
    Renderer->GlyphCache = GlyphCache;

    Renderer->GlobalBuffer = GDI_Context_Create_Buffer(Renderer->Context, {
        .ByteSize = sizeof(ui_box_shader_global),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT
    });
    if(Renderer->GlobalBuffer.Is_Null()) {
        Assert(false);
        return;
    }

    Renderer->GlobalBindGroup = GDI_Context_Create_Bind_Group(Renderer->Context, {
        .Layout = Renderer->Pipeline.BindGroupLayout,
        .WriteInfo = { 
            .Bindings = { { 
                    .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                    .BufferBinding = {
                        .Buffer = Renderer->GlobalBuffer
                    }
                }, {
                    .Type = GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
                    .CopyBinding = {
                        .SrcBindGroup = Renderer->GlyphCache->Atlas.BindGroup
                    }
                }
            }
        }
    });
}

void UI_Renderer_Update(ui_renderer* Renderer, gdi_cmd_list* CmdList) {
    //Rebind the resources to the bind group if they need to change. This only needs to 
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
            GDI_Context_Delete_Buffer(Renderer->Context, Renderer->InstanceBuffer);
            Renderer->InstanceBuffer = {};
        }

        Renderer->InstanceCount = DrawBoxes.Count;
        Renderer->InstanceBuffer = GDI_Context_Create_Buffer(Renderer->Context, {
            .ByteSize   = Renderer->InstanceCount*sizeof(ui_box_shader_box),
            .UsageFlags = GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_GPU_LOCAL_BUFFER_BIT 
        });
    }

    ui_box_shader_global ShaderGlobal = {
        .InvRes = 1.0f / vec2(Renderer->FramebufferDim),
        .InvTexRes = 1.0f / vec2(Renderer->GlyphCache->Atlas.Dim)
    };

    GDI_Context_Buffer_Write(Renderer->Context, Renderer->GlobalBuffer, const_buffer(&ShaderGlobal));
    GDI_Context_Buffer_Write(Renderer->Context, Renderer->InstanceBuffer, const_buffer(span(DrawBoxes)));

    GDI_Cmd_List_Begin_Render_Pass(CmdList, {
        .RenderPass = Renderer->RenderPass.RenderPass,
        .Framebuffer = Renderer->Framebuffer,
        .ClearValues = {
            gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
        }
    });

    GDI_Cmd_List_Set_Pipeline(CmdList, Renderer->Pipeline.Pipeline);
    GDI_Cmd_List_Set_Bind_Groups(CmdList, 0, {Renderer->GlobalBindGroup});
    GDI_Cmd_List_Set_Vtx_Buffers(CmdList, {Renderer->InstanceBuffer});
    GDI_Cmd_List_Draw_Instance(CmdList, 4, Safe_U32(DrawBoxes.Count), 0, 0);

    GDI_Cmd_List_End_Render_Pass(CmdList);
}

void UI_Renderer_Set_Framebuffer(ui_renderer* Renderer, gdi_handle<gdi_framebuffer> Framebuffer, uvec2 FramebufferDim) {
    Renderer->FramebufferDim = FramebufferDim;
    Renderer->Framebuffer = Framebuffer;
}

void UI_Renderer_Set_UI(ui_renderer* Renderer, ui* UI) {
    Renderer->UI = UI;
}