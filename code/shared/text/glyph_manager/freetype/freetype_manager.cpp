#include "freetype_manager.h"

internal void* FT_Alloc(FT_Memory Memory, long Size) {
    allocator* Allocator = (allocator*)Memory->user;
    void* Result = Allocator_Allocate_Memory(Allocator, (uptr)Size);
    return Result;
}

internal void* FT_Realloc(FT_Memory Memory, long CurrentSize, long NewSize, void* OldBlock) {
    if(CurrentSize == NewSize) return OldBlock;

    allocator* Allocator = (allocator*)Memory->user;
    void* Result = Allocator_Allocate_Memory(Allocator, (uptr)NewSize);
    if(Result) {
        if(OldBlock && CurrentSize) {
            Memory_Copy(Result, OldBlock, (uptr)Min(NewSize, CurrentSize));
            Allocator_Free_Memory(Allocator, OldBlock);
        }

        return Result;
    }
    return nullptr;
}

internal void FT_Free(FT_Memory Memory, void* Block) {
    allocator* Allocator = (allocator*)Memory->user;
    if(Memory) {
        Allocator_Free_Memory(Allocator, Block);
    }
}

glyph_manager* Glyph_Manager_Create(const glyph_manager_create_info& CreateInfo) {
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    glyph_manager* Manager = Arena_Push_Struct(Arena, glyph_manager);
    Manager->Arena = Arena;
    Manager->Heap = Heap_Create(Core_Get_Base_Allocator());
    Manager->AsyncAllocator = Lock_Allocator_Create(Manager->Heap);

    FT_Memory Memory = Arena_Push_Struct(Manager->Arena, FT_MemoryRec_);
    Memory->alloc   = FT_Alloc;
    Memory->free    = FT_Free;
    Memory->realloc = FT_Realloc;
    Memory->user    = Manager->AsyncAllocator;

    FT_New_Library(Memory, &Manager->Library);
    FT_Add_Default_Modules(Manager->Library);
    FT_Set_Default_Properties(Manager->Library);

    Async_Pool_Create(&Manager->Faces, Manager->Arena, CreateInfo.MaxFaceCount);

    return Manager;
}

static glyph_face* Glyph_Face_Get(glyph_face_id FaceID) {
    if(!FaceID.ID || !FaceID.Manager) return nullptr;
    return Async_Pool_Get(&FaceID.Manager->Faces, async_handle<glyph_face>(FaceID.ID));
}

glyph_face_id Glyph_Manager_Create_Face(glyph_manager* Manager, const_buffer Buffer, u32 PixelSize) {
    async_handle<glyph_face> FaceHandle = Async_Pool_Allocate(&Manager->Faces);
    if(FaceHandle.Is_Null()) {
        return {};
    }

    glyph_face* Face = Async_Pool_Get(&Manager->Faces, FaceHandle);
    
    FT_Error Error = FT_New_Memory_Face(Manager->Library, (const FT_Byte*)Buffer.Ptr, (FT_Long)Buffer.Size, 0, &Face->Face);
    if(Error != 0) {
        Async_Pool_Free(&Manager->Faces, FaceHandle);
        return {};
    }
    Face->FontBuffer = Buffer;
    FT_Set_Pixel_Sizes(Face->Face, PixelSize, PixelSize);

    return {
        .ID = FaceHandle.ID,
        .Manager = Manager
    };
}

const_buffer Glyph_Face_Get_Font_Buffer(glyph_face_id FaceID) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};
    return Face->FontBuffer;
}

glyph_face_metrics Glyph_Face_Get_Metrics(glyph_face_id FaceID) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};
    FT_Size_Metrics* Metrics = &Face->Face->size->metrics;

    s32 Ascender = Metrics->ascender;
    s32 Descender = Metrics->descender;
    s32 LineGap = Metrics->height - (Ascender-Descender);

    return {
        .Ascender = (u32)(Abs(Ascender) >> 6),
        .Descender = (u32)(Abs(Descender) >> 6),
        .LineGap = (u32)(Abs(LineGap) >> 6)
    };
}

glyph_metrics Glyph_Face_Get_Glyph_Metrics(glyph_face_id FaceID, u32 GlyphIndex) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};

    if(FT_Load_Glyph(Face->Face, GlyphIndex, FT_LOAD_DEFAULT)) {
        return {};
    }

    FT_Glyph_Metrics* GlyphMetric = &Face->Face->glyph->metrics;

    return {
        .Advance = uvec2((u32)(GlyphMetric->horiAdvance >> 6), (u32)(GlyphMetric->vertAdvance >> 6)),
        .Offset = svec2(GlyphMetric->horiBearingX >> 6, GlyphMetric->horiBearingY >> 6),
        .Dim = uvec2((u32)(GlyphMetric->width >> 6), (u32)(GlyphMetric->height >> 6)) 
    };
}

svec2 Glyph_Face_Get_Kerning(glyph_face_id FaceID, u32 GlyphA, u32 GlyphB) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};

    FT_Vector Delta;
    FT_Get_Kerning(Face->Face, GlyphA, GlyphB, FT_KERNING_DEFAULT, &Delta);
    return svec2(Delta.x >> 6, Delta.y >> 6);
}

glyph_bitmap Glyph_Face_Create_Bitmap(glyph_face_id FaceID, allocator* Allocator, u32 GlyphIndex) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};
    
    if(FT_Load_Glyph(Face->Face, GlyphIndex, FT_LOAD_DEFAULT|FT_LOAD_RENDER) != 0) {
        return {};
    }

    FT_GlyphSlot GlyphSlot = Face->Face->glyph;

    glyph_bitmap Result = {
        .Dim = uvec2(GlyphSlot->bitmap.width, GlyphSlot->bitmap.rows)
    };

    switch(GlyphSlot->bitmap.pixel_mode) {
        case FT_PIXEL_MODE_GRAY: {
            Result.Format = GLYPH_BITMAP_FORMAT_GREYSCALE;
            
            uptr DataSize = Result.Dim.w*Result.Dim.y;
            u8* Texels = (u8*)Allocator_Allocate_Memory(Allocator, DataSize);

            u8* DstTexels = Texels;
            const u8* SrcTexels = GlyphSlot->bitmap.buffer;
            for(u32 YIndex = 0; YIndex < Result.Dim.w; YIndex++) {
                for(u32 XIndex = 0; XIndex < Result.Dim.h; XIndex++) {
                    //Premultiplied alpha, assign each pixel the alpha channel
                    *DstTexels++ = *SrcTexels++;
                }
            }

            Result.Texels = const_buffer(Texels, DataSize);
        } break;

        Invalid_Default_Case();
    }

    return Result;
}

glyph_face* FT_Glyph_Face_Get(glyph_face_id FaceID) {
    return Glyph_Face_Get(FaceID);
}