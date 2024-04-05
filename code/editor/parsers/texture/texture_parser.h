#ifndef TEXTURE_PARSER_H
#define TEXTURE_PARSER_H

struct texture {
    allocator* Allocator;
    gdi_format Format;
    u8*        Texels;
    u32        Width;
    u32        Height;
};

texture* Texture_Load_From_Path(allocator* Allocator, string Path);
void     Texture_Delete(texture* Texture);
uptr     Texture_Byte_Size(texture* Texture);

#endif