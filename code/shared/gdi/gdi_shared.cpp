bool GDI_Is_Depth_Format(gdi_format Format) {
    local_persist bool Flags[] = {
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

bool GDI_Is_Bind_Group_Buffer(gdi_bind_group_type Type) {
    local_persist const bool IsBindGroupBuffer[] = {
        true,
        true
    };
    static_assert(Array_Count(IsBindGroupBuffer) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return IsBindGroupBuffer[Type];
}

bool GDI_Is_Bind_Group_Dynamic(gdi_bind_group_type Type) {
    local_persist const bool IsBindGroupDynamic[] = {
        false,
        true
    };
    static_assert(Array_Count(IsBindGroupDynamic) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return IsBindGroupDynamic[Type];
}

gdi_render_pass_attachment gdi_render_pass_attachment::Color(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp) {
    return {
        .Type = gdi_render_pass_attachment_type::Color,
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