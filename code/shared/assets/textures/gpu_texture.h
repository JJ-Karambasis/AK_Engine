#ifndef GPU_TEXTURE_H
#define GPU_TEXTURE_H

struct gpu_texture {
    gdi_format                   Format;
    gdi_handle<gdi_texture>      Handle;
    gdi_handle<gdi_texture_view> View;
    gdi_handle<gdi_bind_group>   BindGroup;
    uvec2                        Dim;
};

struct gpu_texture_create_info {
    bool                              IsSRGB = false;
    gdi_format                        Format;
    uvec2                             Dim;
    const_buffer                      Texels;
    gdi_handle<gdi_bind_group_layout> BindGroupLayout;
};

gpu_texture GPU_Texture_Create(gdi_context* Context, const gpu_texture_create_info& CreateInfo);
void        GPU_Texture_Delete(gdi_context* Context, gpu_texture* Texture);

#endif