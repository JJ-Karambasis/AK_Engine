bool GDI_Is_Depth_Format(gdi_format Format) {
    local_persist const bool Flags[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
    };
    static_assert(Array_Count(Flags) == GDI_FORMAT_COUNT);
    Assert(Format < GDI_FORMAT_COUNT);
    return Flags[Format];
}

uptr GDI_Get_Bytes_Per_Pixel(gdi_format Format) {
    local_persist const uptr BytesPerPixel[] = {
        0, 
        1, 
        1,
        2,
        2, 
        3,
        3, 
        4, 
        4, 
        4, 
        4, 
        8, 
        2, 
        4, 
        4, 
        8, 
        12, 
        2
    };
    static_assert(Array_Count(BytesPerPixel) == GDI_FORMAT_COUNT);
    Assert(Format < GDI_FORMAT_COUNT);
    return BytesPerPixel[Format];
}

gdi_format GDI_Get_SRGB_Format(gdi_format Format) {
    local_persist const gdi_format Formats[] = {
        GDI_FORMAT_NONE, 
        GDI_FORMAT_R8_SRGB, 
        GDI_FORMAT_R8_SRGB, 
        GDI_FORMAT_R8G8_SRGB, 
        GDI_FORMAT_R8G8_SRGB, 
        GDI_FORMAT_R8G8B8_SRGB, 
        GDI_FORMAT_R8G8B8_SRGB, 
        GDI_FORMAT_R8G8B8A8_SRGB, 
        GDI_FORMAT_R8G8B8A8_SRGB, 
        GDI_FORMAT_B8G8R8A8_SRGB, 
        GDI_FORMAT_B8G8R8A8_SRGB, 
        GDI_FORMAT_NONE, 
        GDI_FORMAT_NONE, 
        GDI_FORMAT_NONE, 
        GDI_FORMAT_NONE,
        GDI_FORMAT_NONE,
        GDI_FORMAT_NONE,
        GDI_FORMAT_NONE,
    };
    static_assert(Array_Count(Formats) == GDI_FORMAT_COUNT);
    Assert(Format < GDI_FORMAT_COUNT);
    return Formats[Format];
}

bool GDI_Is_Bind_Group_Buffer(gdi_bind_group_type Type) {
    local_persist const bool IsBindGroupBuffer[] = {
        true,
        true,
        false,
        false
    };
    static_assert(Array_Count(IsBindGroupBuffer) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return IsBindGroupBuffer[Type];
}

bool GDI_Is_Bind_Group_Dynamic(gdi_bind_group_type Type) {
    local_persist const bool IsBindGroupDynamic[] = {
        false,
        true,
        false,
        false
    };
    static_assert(Array_Count(IsBindGroupDynamic) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return IsBindGroupDynamic[Type];
}

bool GDI_Is_Bind_Group_Texture(gdi_bind_group_type Type) {
    local_persist const bool IsBindGroupTexture[] = {
        false,
        false,
        true,
        false
    };
    static_assert(Array_Count(IsBindGroupTexture) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return IsBindGroupTexture[Type];
}

gdi_render_pass_attachment gdi_render_pass_attachment::Color(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp) {
    return {
        .Type = gdi_render_pass_attachment_type::Color,
        .Format = Format,
        .LoadOp = LoadOp,
        .StoreOp = StoreOp
    };
}

gdi_render_pass_attachment gdi_render_pass_attachment::Depth(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp) {
    return {
        .Type = gdi_render_pass_attachment_type::Depth,
        .Format = Format,
        .LoadOp = LoadOp,
        .StoreOp = StoreOp
    };
}

gdi_resource gdi_resource::Texture(gdi_handle<gdi_texture> Texture) {
    return {
        .Type = gdi_resource_type::Texture,
        .TextureHandle = Texture
    };
}

gdi_clear gdi_clear::Color(f32 r, f32 g, f32 b, f32 a) {
    return {
        .Type = gdi_clear_type::Color,
        .ClearColor = {r, g, b, a}
    };
}

gdi_clear gdi_clear::Depth(f32 Depth) {
    return {
        .Type = gdi_clear_type::Depth,
        .ClearDepth = {Depth}
    };
}

#include "gdi_memory.cpp"