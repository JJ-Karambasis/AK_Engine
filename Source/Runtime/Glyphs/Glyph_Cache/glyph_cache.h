#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#define GLYPH_CACHE_SLOT_CAPACITY 1024
#define GLYPH_CACHE_SLOT_MASK (GLYPH_CACHE_SLOT_CAPACITY-1)
#define GLYPH_CACHE_GLYPHS_PER_BUCKET 64

typedef struct glyph_cache_entry glyph_cache_entry;

typedef struct glyph_cache_entry
{
    glyph              Glyph;
    uint64_t           Version;
    font_face*         Face;
    uint32_t           SlotIndex;
    uint32_t           Hash;
    glyph_cache_entry* NextSlotEntry;
    glyph_cache_entry* PrevSlotEntry;
    glyph_cache_entry* NextLRUEntry;
    glyph_cache_entry* PrevLRUEntry;
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

typedef struct glyph_cache_lru
{
    glyph_cache_entry* First;
    glyph_cache_entry* Last;
} glyph_cache_lru;

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
    glyph_generator*      Generator;
    arena*                Arena;
    glyph_slot*           Slots;
    uint64_t              BucketCount;
    glyph_cache_bucket*   CurrentBucket;
    glyph_cache_lru       LRU;
    glyph_generate_entry* FreeGenerateEntries;
    glyph_generate_queue  GlyphGenerateQueue;
    uint64_t              GlyphCount;
    uint64_t              Version;
} glyph_cache;

glyph_cache* Glyph_Cache_Create(allocator* Allocator, glyph_generator* Generator);
glyph*       Glyph_Cache_Get(glyph_cache* Cache, font_face* Face, uint32_t Codepoint);
void         Glyph_Cache_Generate(glyph_cache* Cache);
void         Glyph_Cache_Delete(glyph_cache* Cache);

#endif