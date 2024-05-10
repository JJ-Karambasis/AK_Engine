template <>
struct hasher<glyph_cache_key> {
    inline u32 Hash(glyph_cache_key Key) {
        u64 Packed = ((((u64)Key.Codepoint) << 32) | ((u64)Key.Size));
        u32 Hash = Hash_Bytes(Key.FontBufferPtr, sizeof(void*));
        Hash = Hash_Combine(Hash, Hash_U64(Packed));
        return Hash;
    }
};

template <>
struct comparer<glyph_cache_key> {
    inline bool Equal(glyph_cache_key A, glyph_cache_key B) {
        return A.FontBufferPtr == B.FontBufferPtr && A.Size == B.Size && A.Codepoint == B.Codepoint;
    }
};

glyph_cache* Glyph_Cache_Create(const glyph_cache_create_info& CreateInfo) {
    arena* Arena = Arena_Create(CreateInfo.Allocator);
    glyph_cache* Result = Arena_Push_Struct(Arena, glyph_cache);
    Result->Arena    = Arena;
    Result->Renderer = CreateInfo.Renderer;

    Result->Atlas = Renderer_Texture_Create(Result->Renderer, {
        .IsSRGB          = true,
        .Format          = GDI_FORMAT_R8G8B8A8_UNORM,
        .Dim             = CreateInfo.AtlasDim,
    });

    gdi_context* Context = Renderer_Get_Context(Result->Renderer);

    Result->AtlasAllocator = Atlas_Allocator_Create({
        .Allocator = Result->Arena, 
        .Dim = CreateInfo.AtlasDim
    });
    atlas_alloc_id FirstWhitespace = Atlas_Allocator_Alloc(Result->AtlasAllocator, uvec2(2, 2));

    u32 Texels[] = {
        0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF
    };
    GDI_Context_Upload_Texture(Context, Result->Atlas.Handle, {
        { const_buffer(Texels), 0, 0, 2, 2}
    });

    ak_slot64* CreateEntrySlots = Arena_Push_Array(Result->Arena, CreateInfo.MaxCreateEntryCount, ak_slot64);
    u32* CreateEntryIndices = Arena_Push_Array(Result->Arena, CreateInfo.MaxCreateEntryCount, u32);
    AK_Async_Slot_Map64_Init_Raw(&Result->CreateEntrySlots, CreateEntryIndices, CreateEntrySlots, CreateInfo.MaxCreateEntryCount);
    Result->CreateEntries = Arena_Push_Array(Result->Arena, CreateInfo.MaxCreateEntryCount, glyph_cache_create_entry);

    ak_slot64* UpdateEntrySlots = Arena_Push_Array(Result->Arena, CreateInfo.MaxUpdateEntryCount, ak_slot64);
    u32* UpdateEntryIndices = Arena_Push_Array(Result->Arena, CreateInfo.MaxUpdateEntryCount, u32);
    AK_Async_Slot_Map64_Init_Raw(&Result->UpdateEntrySlots, UpdateEntryIndices, UpdateEntrySlots, CreateInfo.MaxUpdateEntryCount);
    Result->UpdateEntries = Arena_Push_Array(Result->Arena, CreateInfo.MaxUpdateEntryCount, glyph_cache_update_entry);

    Hashmap_Init(&Result->Cache, Result->Arena);
    Result->UpdateCount = 1;

    return Result;
}

void               Glyph_Cache_Delete(glyph_cache* Cache);

const renderer_texture* Glyph_Cache_Get_Atlas(glyph_cache* Cache) {
    return &Cache->Atlas;
}

const glyph_entry* Glyph_Cache_Get(glyph_cache* Cache, font_id Font, u32 Codepoint) {
    //Hash by the font buffer instead of faces because, we may duplicate faces
    //for thread safety
    const_buffer FontBuffer = Font_Get_Buffer(Font);
    u32 Size = Font_Get_Size(Font); 

    glyph_cache_key Key = {
        .FontBufferPtr = FontBuffer.Ptr,
        .Size = Size,
        .Codepoint = Codepoint
    };

    u32 Hash = hasher<glyph_cache_key>{}.Hash(Key);
    glyph_cache_entry** pEntry = Hashmap_Find_By_Hash(&Cache->Cache, Key, Hash);

    if(pEntry) {
        glyph_cache_entry* Entry = *pEntry;
        if(AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(&Entry->Updated, false, true)) {
            ak_slot64 Slot = AK_Async_Slot_Map64_Alloc_Slot(&Cache->UpdateEntrySlots);
            if(!Slot) {
                //Cache is empty!
                Assert(false);
                return nullptr;
            }
            AK_Atomic_Store_U64_Relaxed(&Entry->UpdateIndex, Cache->UpdateCount);

            glyph_cache_update_entry* UpdateEntry = Cache->UpdateEntries + AK_Slot64_Index(Slot);
            UpdateEntry->Slot  = Slot;
            UpdateEntry->Entry = Entry;
            UpdateEntry->Next  = nullptr;
            
            SLL_Push_Front_Async(&Cache->GlyphsToUpdateList, UpdateEntry);
        }

        return Entry;
    } else {
        ak_slot64 Slot = AK_Async_Slot_Map64_Alloc_Slot(&Cache->CreateEntrySlots);
        if(!Slot) {
            //Cache is empty!
            Assert(false);
            return nullptr;
        }

        glyph_cache_create_entry* Entry = Cache->CreateEntries + AK_Slot64_Index(Slot);
        Entry->Slot      = Slot;
        Entry->Font      = Font;
        Entry->Size      = Size;
        Entry->Codepoint = Codepoint;
        Entry->Hash      = Hash;
        Entry->Next      = nullptr;

        SLL_Push_Front_Async(&Cache->GlyphsToCreateList, Entry);
        return nullptr;
    }
}

void Glyph_Cache_Update(glyph_cache* Cache) {
    glyph_cache_update_entry* UpdateEntry = (glyph_cache_update_entry*)AK_Atomic_Load_Ptr_Relaxed(&Cache->GlyphsToUpdateList);
    while(UpdateEntry) {
        //Updating an entry is very simple. Just make sure to remove it 
        //from the lru list and add it back to the front. 
        //Glyph_Cache_Get will already handle setting the update index
        glyph_cache_entry* Entry = UpdateEntry->Entry;
        Entry->Updated.Nonatomic = false;
        DLL_Remove_NP(Cache->LRUHead, Cache->LRUTail, Entry, NextLRU, PrevLRU);
        DLL_Push_Front_NP(Cache->LRUHead, Cache->LRUTail, Entry, NextLRU, PrevLRU);
        
        ak_slot64 Slot           = UpdateEntry->Slot;
        UpdateEntry              = UpdateEntry->Next;
        AK_Async_Slot_Map64_Free_Slot(&Cache->UpdateEntrySlots, Slot);
    }
    AK_Atomic_Store_Ptr_Relaxed(&Cache->GlyphsToUpdateList, nullptr);

    scratch Scratch = Scratch_Get();
    array<gdi_texture_upload> TextureCopies(&Scratch);

    glyph_cache_create_entry* CreateEntry = (glyph_cache_create_entry*)AK_Atomic_Load_Ptr_Relaxed(&Cache->GlyphsToCreateList);
    while(CreateEntry) {
        //First check to make sure the create entry isn't a duplicate.
        //This is totally possible with multithreaded Glyph_Cache_Gets
        const_buffer FontBuffer = Font_Get_Buffer(CreateEntry->Font);
        glyph_cache_key Key = {
            .FontBufferPtr = FontBuffer.Ptr,
            .Size = CreateEntry->Size,
            .Codepoint = CreateEntry->Codepoint
        };
        if(!Hashmap_Find_By_Hash(&Cache->Cache, Key, CreateEntry->Hash)) {
            //Then allocate a bitmap from the face with the appropriate size
            glyph_bitmap Bitmap = Font_Rasterize_Glyph(CreateEntry->Font, &Scratch, CreateEntry->Codepoint);
            if(Bitmap.Texels.Ptr) {
                if(Bitmap.Dim.x != 0 && Bitmap.Dim.y != 0) {
                    //Next we will try to allocate out of the current atlas allocator
                    atlas_alloc_id AtlasAlloc = Atlas_Allocator_Alloc(Cache->AtlasAllocator, Bitmap.Dim);
                    atlas_index* AtlasIndex = Atlas_Allocator_Get(Cache->AtlasAllocator, AtlasAlloc);

                    if(!AtlasIndex) {
                        //If we fail to allocate out of the atlas that means the cache
                        //is now full. Lets free all atlas data that is old
                        glyph_cache_entry* OldestEntry = Cache->LRUTail;
                        while(OldestEntry && (OldestEntry->UpdateIndex.Nonatomic < Cache->UpdateCount)) {
                            Atlas_Allocator_Free(Cache->AtlasAllocator, OldestEntry->AllocID);
                            Hashmap_Remove_By_Hash(&Cache->Cache, OldestEntry->Key, OldestEntry->Hash);
                            DLL_Remove_Back_NP(Cache->LRUHead, Cache->LRUTail, NextLRU, PrevLRU);
                            SLL_Push_Front_N(Cache->FreeEntries, OldestEntry, NextLRU);
                            OldestEntry = Cache->LRUTail;
                        }

                        AtlasAlloc = Atlas_Allocator_Alloc(Cache->AtlasAllocator, Bitmap.Dim);
                        AtlasIndex = Atlas_Allocator_Get(Cache->AtlasAllocator, AtlasAlloc);
                    }
                    
                    //If we succeeded allocating out of the atlas we are done! 
                    if(AtlasIndex) {
                        //Allocate an entry and correctly fill out the uvs and add
                        //to the hashmap and lru list
                        glyph_cache_entry* Entry = Cache->FreeEntries;
                        if(Entry) SLL_Pop_Front_N(Cache->FreeEntries, NextLRU);
                        else Entry = Arena_Push_Struct(Cache->Arena, glyph_cache_entry);
                        Zero_Struct(Entry);

                        //Convert texels from bitmap to SRGB
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


                        Array_Push(&TextureCopies, {
                            .Texels  = Texels,
                            .XOffset = AtlasIndex->Rect.Min.x,
                            .YOffset = AtlasIndex->Rect.Min.y,
                            .Width   = Bitmap.Dim.w,
                            .Height  = Bitmap.Dim.h
                        });

                        Entry->AllocID = AtlasAlloc;
                        Entry->Key = Key;
                        Entry->Hash = CreateEntry->Hash;
                        Entry->AtlasRect = AtlasIndex->Rect;

                        Hashmap_Add_By_Hash(&Cache->Cache, {
                            .FontBufferPtr = FontBuffer.Ptr,
                            .Size = CreateEntry->Size,
                            .Codepoint = CreateEntry->Codepoint
                        }, CreateEntry->Hash, Entry);
                        
                        DLL_Push_Front_NP(Cache->LRUHead, Cache->LRUTail, Entry, NextLRU, PrevLRU);
                        Entry->UpdateIndex.Nonatomic = Cache->UpdateCount;
                    } else {
                        //If we still fail to allocate the atlas that really means
                        //our atlas is full now. 
                        //todo: We should probably resize the atlas to something greater
                        //then. If its larger than a maximum size then we have a cache that is
                        //full and we can't cache anymore glyphs
                        Not_Implemented();
                    }
                }
            }
        }

        ak_slot64 Slot = CreateEntry->Slot;
        CreateEntry    = CreateEntry->Next;
        AK_Async_Slot_Map64_Free_Slot(&Cache->CreateEntrySlots, Slot);
    }
    if(TextureCopies.Count) {
        gdi_context* Context = Renderer_Get_Context(Cache->Renderer);
        GDI_Context_Upload_Texture(Context, Cache->Atlas.Handle, TextureCopies);
    }

    AK_Atomic_Store_Ptr_Relaxed(&Cache->GlyphsToCreateList, nullptr);
    Cache->UpdateCount++;
}