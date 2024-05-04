#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

struct glyph_entry {
    rect2u AtlasRect;
};

struct glyph_cache_key {
    const void* FontBufferPtr;
    u32         Size;
    u32         Codepoint;
};

struct glyph_cache_entry : glyph_entry {
    glyph_cache_entry* PrevLRU;
    glyph_cache_entry* NextLRU;
    atlas_alloc_id     AllocID;
    glyph_cache_key    Key;
    u32                Hash;
    ak_atomic_u32      Updated;
    ak_atomic_u64      UpdateIndex;
};

struct glyph_cache_create_entry {
    ak_slot64                 Slot;
    font_id                   Font;
    u32                       Size;
    u32                       Codepoint;
    u32                       Hash;
    glyph_cache_create_entry* Next;
};

struct glyph_cache_update_entry {
    ak_slot64                 Slot;
    glyph_cache_entry*        Entry;
    glyph_cache_update_entry* Next;
};

struct glyph_cache {
    arena*                                       Arena;
    gdi_context*                                 Context;
    gdi_handle<gdi_bind_group_layout>            AtlasLayout;
    gpu_texture                                  Atlas;
    atlas_allocator*                             AtlasAllocator;
    ak_async_slot_map64                          CreateEntrySlots;
    glyph_cache_create_entry*                    CreateEntries;
    ak_async_slot_map64                          UpdateEntrySlots;
    glyph_cache_update_entry*                    UpdateEntries;
    hashmap<glyph_cache_key, glyph_cache_entry*> Cache;
    glyph_cache_entry*                           LRUHead;
    glyph_cache_entry*                           LRUTail;
    glyph_cache_entry*                           FreeEntries;
    ak_atomic_ptr                                GlyphsToUpdateList;
    ak_atomic_ptr                                GlyphsToCreateList;
    u64                                          UpdateCount;
};

struct glyph_cache_create_info {
    gdi_context*                      Context;
    allocator*                        Allocator             = Core_Get_Base_Allocator();
    uvec2                             AtlasDim              = uvec2(1024, 1024);
    u32                               MaxUpdateEntryCount   = 1024;
    u32                               MaxCreateEntryCount   = 1024;
};

glyph_cache*       Glyph_Cache_Create(const glyph_cache_create_info& CreateInfo);
void               Glyph_Cache_Delete(glyph_cache* Cache);

//Glyph_Cache_Get is threadsafe relative to other Glyph_Cache_Gets,
//must call Glyph_Cache_Update when no more Glyph_Cache_Gets are being
//called so we can update the cache
const glyph_entry* Glyph_Cache_Get(glyph_cache* Cache, font_id Font, u32 Codepoint);
void               Glyph_Cache_Update(glyph_cache* Cache);


#endif