typedef uint64_t font_key;
#define Font_Key_Make(codepoint, size) ((((u64)(codepoint)) << 32) | ((u64)(size)))
#define Font_Key_Codepoint(key) (u32)((key) >> 32)
#define Font_Key_Size(key) (u32)(key)

internal gdi_format Font_Get_GDI_Format(glyph_bitmap_format Format) {
    local_persist const gdi_format Formats[] = {
        GDI_FORMAT_R8_UNORM
    };
    static_assert(Array_Count(Formats) == GLYPH_BITMAP_FORMAT_COUNT);
    Assert(Format < GLYPH_BITMAP_FORMAT_COUNT);
    return Formats[Format];
}

struct glyph_cache {
    glyph_face*                       Face;
    gdi_context*                      GDIContext;
    gdi_handle<gdi_bind_group_layout> BitmapBindGroupLayout;
    ak_rw_lock                        RWLock;
    hashmap<font_key, font_bitmap>    Cache;
};

internal void Glyph_Cache_Init(glyph_cache* Cache, gdi_context* GDIContext, gdi_handle<gdi_bind_group_layout> BitmapBindGroupLayout, 
                               lock_allocator* Allocator, glyph_face* Face) {
    Cache->Face = Face;
    Cache->GDIContext = GDIContext;
    Cache->BitmapBindGroupLayout = BitmapBindGroupLayout;
    AK_RW_Lock_Create(&Cache->RWLock);
    Hashmap_Init(&Cache->Cache, Allocator); 
}

internal font_bitmap Glyph_Cache_Get_Or_Add(glyph_cache* Cache, font_key Key) {
    font_bitmap Result = {};

    //Make sure we copy to the main bitmap inside the lock
    //just in case we add to the hashmap and the pointer is 
    //invalidated
    AK_RW_Lock_Reader(&Cache->RWLock);
    font_bitmap* pBitmap = Hashmap_Find(&Cache->Cache, Key);
    if(pBitmap) Result = *pBitmap;
    AK_RW_Unlock_Reader(&Cache->RWLock);

    if(Result.Texture.Handle.Is_Null()) {
        scratch Scratch = Scratch_Get();
        Glyph_Face_Set_Size(Cache->Face, Font_Key_Size(Key));
        glyph_bitmap Bitmap = Glyph_Face_Create_Bitmap(Cache->Face, &Scratch, Font_Key_Codepoint(Key));
        if(Bitmap.Texels.Ptr) {
            //All fonts must be in RGBA format for consistent rendering

            const_buffer Texels = {};
            switch(Bitmap.Format) {
                case GLYPH_BITMAP_FORMAT_GREYSCALE: {
                    void* NewTexels = Scratch_Push(&Scratch, Bitmap.Dim.w*Bitmap.Dim.h*4);
                    
                    u8* DstTexels = (u8*)NewTexels;
                    const u8* SrcTexels = Bitmap.Texels.Ptr;

                    for(u32 YIndex = 0; YIndex < Bitmap.Dim.h; YIndex++) {
                        for(u32 XIndex = 0; XIndex < Bitmap.Dim.w; XIndex++) {
                            //Premultiplied alpha, assign each pixel the alpha channel
                            *DstTexels++ = *SrcTexels;
                            *DstTexels++ = *SrcTexels;
                            *DstTexels++ = *SrcTexels;
                            *DstTexels++ = *SrcTexels;
                            SrcTexels++;
                        }
                    }

                    Texels = const_buffer(NewTexels, Bitmap.Dim.w*Bitmap.Dim.h*4);
                } break;

                Invalid_Default_Case();
            }

            Result.Texture = GPU_Texture_Create(Cache->GDIContext, {
                .IsSRGB = true,
                .Format = GDI_FORMAT_R8G8B8A8_UNORM,
                .Dim = Bitmap.Dim,
                .Texels = Texels,
                .BindGroupLayout = Cache->BitmapBindGroupLayout
            });

            AK_RW_Lock_Writer(&Cache->RWLock);
            Hashmap_Add(&Cache->Cache, Key, Result);
            AK_RW_Unlock_Writer(&Cache->RWLock);
        }
    }

    return Result;
}

struct font {
    heap*           Heap;
    lock_allocator* AsyncAllocator;
    const_buffer    FontBuffer;
    glyph_cache     GlyphCache;
};

font* Font_Create(const font_create_info& CreateInfo) {
    uptr AllocationSize = sizeof(font)+CreateInfo.FontBuffer.Size;
    heap* Heap = Heap_Create(Core_Get_Base_Allocator());
    font* Font = (font*)Allocator_Allocate_Memory(Heap, AllocationSize);
    void* Data = (void*)(Font+1);
    Memory_Copy(Data, CreateInfo.FontBuffer.Ptr, CreateInfo.FontBuffer.Size);

    Font->Heap = Heap;
    Font->AsyncAllocator = Lock_Allocator_Create(Font->Heap);
    Font->FontBuffer = const_buffer(Data, CreateInfo.FontBuffer.Size);
    
    glyph_face* Face = Glyph_Manager_Create_Face(CreateInfo.GlyphManager, Font->FontBuffer);
    Glyph_Cache_Init(&Font->GlyphCache, CreateInfo.GDIContext, CreateInfo.BitmapBindGroupLayout, Font->AsyncAllocator, Face);
    return Font;
}

font_bitmap Font_Get_Bitmap(font* Font, u32 Codepoint, u32 Size) {
    font_bitmap FontBitmap = Glyph_Cache_Get_Or_Add(&Font->GlyphCache, Font_Key_Make(Codepoint, Size));
    return FontBitmap;
}
