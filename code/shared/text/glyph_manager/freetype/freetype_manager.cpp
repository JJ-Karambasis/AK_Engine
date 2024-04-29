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

    ak_slot64* Slots = Arena_Push_Array(Manager->Arena, CreateInfo.MaxFaceCount, ak_slot64);
    u32* FreeIndices = Arena_Push_Array(Manager->Arena, CreateInfo.MaxFaceCount, u32);
    AK_Async_Slot_Map64_Init_Raw(&Manager->FaceSlots, FreeIndices, Slots, CreateInfo.MaxFaceCount);
    Manager->Faces = Arena_Push_Array(Manager->Arena, CreateInfo.MaxFaceCount, glyph_face);

    return Manager;
}

static glyph_face* Glyph_Face_Get(glyph_face_id FaceID) {
    if(!FaceID.Generation || !FaceID.Face) return nullptr;
    return AK_Async_Slot_Map64_Is_Allocated(&FaceID.Face->Manager->FaceSlots, (ak_slot64)FaceID.Generation) ? FaceID.Face : nullptr;
}

glyph_face_id Glyph_Manager_Create_Face(glyph_manager* Manager, const_buffer Buffer) {
    FT_Face Face;

    FT_Error Error = FT_New_Memory_Face(Manager->Library, (const FT_Byte*)Buffer.Ptr, (FT_Long)Buffer.Size, 0, &Face);
    if(Error != 0) {
        return {};
    }

    ak_slot64 Slot = AK_Async_Slot_Map64_Alloc_Slot(&Manager->FaceSlots);
    glyph_face* Result = Manager->Faces + AK_Slot64_Index(Slot);
    Result->Manager = Manager;
    Result->Face = Face;
    Result->FontBuffer = Buffer;

    return {
        .Generation = Slot,
        .Face       = Result
    };
}

const_buffer Glyph_Face_Get_Font_Buffer(glyph_face_id FaceID) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};
    return Face->FontBuffer;
}

u32 Glyph_Face_Get_Size(glyph_face_id FaceID) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return 0;
    return Face->Size;
}

void Glyph_Face_Set_Size(glyph_face_id FaceID, u32 PixelSize) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return;

    FT_Set_Pixel_Sizes(Face->Face, PixelSize, PixelSize);
    Face->Size = PixelSize;
}

glyph_bitmap Glyph_Face_Create_Bitmap(glyph_face_id FaceID, allocator* Allocator, u32 Codepoint) {
    glyph_face* Face = Glyph_Face_Get(FaceID);
    if(!Face) return {};
    
    if(FT_Load_Char(Face->Face, Codepoint, FT_LOAD_DEFAULT|FT_LOAD_RENDER) != 0) {
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