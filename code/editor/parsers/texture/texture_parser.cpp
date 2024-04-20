#ifdef TEXTURE_PARSER_USE_STBI

#ifdef COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4365 5219)
#endif

#include "stbi/stbi_texture_parser.cpp"

#ifdef COMPILER_MSVC
#pragma warning(pop)
#endif

#else
#endif

void Texture_Delete(texture* Texture) {
    if(Texture && Texture->Allocator) {
        allocator* Allocator = Texture->Allocator;
        Texture->Allocator = NULL;
        Allocator_Free_Memory(Allocator, Texture);
    }
}

uptr Texture_Byte_Size(texture* Texture) {
    return Texture->Width*Texture->Height*GDI_Get_Bytes_Per_Pixel(Texture->Format);
}