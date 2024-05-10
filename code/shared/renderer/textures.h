#ifndef TEXTURES_H
#define TEXTURES_H

struct renderer_texture {
    gdi_format                   Format;
    gdi_handle<gdi_texture>      Handle;
    gdi_handle<gdi_texture_view> View;
    gdi_handle<gdi_bind_group>   BindGroup;
    uvec2                        Dim;

    inline bool Is_Null() const {
        return Handle.Is_Null() || View.Is_Null() || BindGroup.Is_Null();
    }
};

struct renderer_texture_create_info {
    bool         IsSRGB = false;
    gdi_format   Format;
    uvec2        Dim;
    const_buffer Texels;
};

renderer_texture Renderer_Texture_Create(renderer* Renderer, const renderer_texture_create_info& CreateInfo);
void             Renderer_Texture_Delete(renderer* Renderer, renderer_texture* Texture);

#endif