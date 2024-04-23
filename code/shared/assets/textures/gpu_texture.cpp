gpu_texture GPU_Texture_Create(gdi_context* Context, const gpu_texture_create_info& CreateInfo) {
    gdi_handle<gdi_texture> Texture = GDI_Context_Create_Texture(Context, {
        .Format = CreateInfo.Format,
        .Width  = CreateInfo.Dim.w,
        .Height = CreateInfo.Dim.y,
        .UsageFlags = GDI_TEXTURE_USAGE_FLAG_SAMPLED_BIT|GDI_TEXTURE_USAGE_FLAG_COPIED_BIT,
        .InitialData = CreateInfo.Texels
    });

    if(Texture.Is_Null()) {
        return {};
    }

    gdi_handle<gdi_texture_view> TextureView = GDI_Context_Create_Texture_View(Context, {
        .Texture = Texture,
        .Format  = CreateInfo.IsSRGB ? GDI_Get_SRGB_Format(CreateInfo.Format) : CreateInfo.Format
    });
    
    if(TextureView.Is_Null()) {
        GDI_Context_Delete_Texture(Context, Texture);
        return {};
    }

    gdi_handle<gdi_bind_group> BindGroup = GDI_Context_Create_Bind_Group(Context, {
        .Layout = CreateInfo.BindGroupLayout,
        .WriteInfo = {
            .Bindings = { { 
                    .Type = GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
                    .TextureBinding = {
                        .TextureView = TextureView
                    }
                }
            }
        }
    });

    if(BindGroup.Is_Null()) {
        GDI_Context_Delete_Texture_View(Context, TextureView);
        GDI_Context_Delete_Texture(Context, Texture);
        return {};
    }

    return {
        .Format = CreateInfo.Format,
        .Handle = Texture,
        .View = TextureView,
        .BindGroup = BindGroup,
        .Dim = CreateInfo.Dim
    };
}

void        GPU_Texture_Delete(gdi_context* Context, gpu_texture* Texture);