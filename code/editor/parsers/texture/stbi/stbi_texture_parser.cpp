#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static gdi_format STBI_Get_Channel_Count_Format(u32 ChannelCount) {
    local_persist const gdi_format ChannelFormats[] = {
        GDI_FORMAT_NONE,
        GDI_FORMAT_R8_UNORM,
        GDI_FORMAT_R8G8_UNORM,
        GDI_FORMAT_R8G8B8A8_UNORM, //channel count of 3 gets padded
        GDI_FORMAT_R8G8B8A8_UNORM
    };
    Assert(ChannelCount < Array_Count(ChannelFormats));
    return ChannelFormats[ChannelCount];
}

texture* Texture_Load_From_Path(allocator* Allocator, string Path) {
    scratch Scratch = Scratch_Get();
    buffer Buffer = OS_Read_Entire_File(&Scratch, Path);
    if(Buffer.Is_Empty()) {
        //todo: Logging
        return NULL;
    }

    int Width, Height, ChannelCount;
    u8* Texels = stbi_load_from_memory(Buffer.Ptr, (int)Buffer.Size, &Width, &Height, &ChannelCount, 0);
    if(!Texels) {
        //todo: Logging
        return NULL;
    }

    if(ChannelCount == 3) {
        uptr AllocationSize = sizeof(texture) + (Width*Height*4);
        texture* Texture = (texture*)Allocator_Allocate_Memory(Allocator, AllocationSize);
        Texture->Allocator = Allocator;
        Texture->Texels = (u8*)(Texture+1);
        Texture->Width = Width;
        Texture->Height = Height;
        Texture->Format = STBI_Get_Channel_Count_Format(ChannelCount);
        
        u8* DstTexels = Texture->Texels;
        const u8* SrcTexels = Texels;
        for(int x = 0; x < Width; x++) {
            for(int y = 0; y < Height; y++) {
                *DstTexels++ = *SrcTexels++;
                *DstTexels++ = *SrcTexels++;
                *DstTexels++ = *SrcTexels++;
                *DstTexels++ = 255;
            }
        }        
        stbi_image_free(Texels);
        return Texture;
    } else {
        uptr AllocationSize = sizeof(texture) + (Width*Height*ChannelCount);
        texture* Texture = (texture*)Allocator_Allocate_Memory(Allocator, AllocationSize);
        Texture->Allocator = Allocator;
        Texture->Texels = (u8*)(Texture+1);
        Texture->Width = Width;
        Texture->Height = Height;
        Texture->Format = STBI_Get_Channel_Count_Format(ChannelCount);
        Memory_Copy(Texture->Texels, Texels, (Width*Height*ChannelCount));
        stbi_image_free(Texels);
        return Texture;
    }
}