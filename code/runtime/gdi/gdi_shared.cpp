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

gdi_render_pass_attachment gdi_render_pass_attachment::Color(gdi_format Format, gdi_load_op LoadOp, gdi_store_op StoreOp) {
    return {
        .Type = gdi_render_pass_attachment_type::Color,
        .Format = Format,
        .LoadOp = LoadOp,
        .StoreOp = StoreOp
    };
}