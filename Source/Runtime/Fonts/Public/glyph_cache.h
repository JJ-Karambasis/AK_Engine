#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#define GLYPH_CACHE_GLYPHS_PER_BUCKET 64

typedef struct glyph_cache_entry glyph_cache_entry;

typedef struct glyph_cache_entry
{
    glyph              Glyph;
    glyph_font*        Font;
    uint32_t           PixelHeight;
    uint32_t           SlotIndex;
    uint32_t           Hash;
    glyph_cache_entry* NextSlotEntry;
    glyph_cache_entry* PrevSlotEntry;
} glyph_cache_entry;

typedef struct glyph_cache_bucket
{
    glyph_cache_entry Glyphs[GLYPH_CACHE_GLYPHS_PER_BUCKET];
    uint32_t          Count;
} glyph_cache_bucket;

typedef struct glyph_slot
{
    glyph_cache_entry* First;
    glyph_cache_entry* Last;
} glyph_slot;

typedef struct glyph_generate_entry
{
    glyph_cache_entry*           CacheEntry;
    struct glyph_generate_entry* Next;
} glyph_generate_entry;

typedef struct glyph_generate_queue
{
    glyph_generate_entry* First;
    glyph_generate_entry* Last;
    uint64_t              Count;
} glyph_generate_queue;

typedef struct glyph_cache
{
    arena*                Arena;
    gpu_resource_manager* ResourceManager;
    uint32_t              SlotCapacity;
    glyph_slot*           Slots;
    uint64_t              BucketCount;
    glyph_cache_bucket*   CurrentBucket;
    glyph_generate_entry* FreeGenerateEntries;
    glyph_generate_queue  GenerateQueue;
    uint32_t              GlyphCount;
} glyph_cache;

glyph_cache*    Glyph_Cache_Create(allocator* Allocator, gpu_resource_manager* ResourceManager, 
                                   uint32_t SlotCapacity);
glyph_info_list Glyph_Cache_Get_Glyphs(glyph_cache* Cache, allocator* Allocator, glyph_font* Font, str8 Text, uint32_t PixelHeight);
void            Glyph_Cache_Generate(glyph_cache* Cache, gpu_cmd_buffer* CmdBuffer);
void            Glyph_Cache_Delete(glyph_cache* Cache);

#endif