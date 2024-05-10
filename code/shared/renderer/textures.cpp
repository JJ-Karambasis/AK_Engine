renderer_texture Renderer_Texture_Create(renderer* Renderer, const renderer_texture_create_info& CreateInfo) {
    gdi_context* Context = Renderer->Context;
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
        .Layout = Renderer->TextureLayout,
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

void Renderer_Texture_Delete(renderer* Renderer, renderer_texture* Texture);