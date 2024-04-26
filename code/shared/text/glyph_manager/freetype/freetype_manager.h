#ifndef FREETYPE_RASTERIZER_H
#define FREETYPE_RASTERIZER_H

//NOTE: Our core implementation defines the internal macro which seems to
//conflict with code in freetype, therefore it needs to be included before
//we include core
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsystem.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftadvanc.h>

#include <core/core.h>
#include <text/text.h>

struct glyph_manager {
    arena*              Arena;
    heap*               Heap;
    lock_allocator*     AsyncAllocator;
    FT_Library          Library;
    ak_async_slot_map64 FaceSlots;
    glyph_face*         Faces;
};

struct glyph_face {
    glyph_manager* Manager;
    FT_Face        Face;
    const_buffer   FontBuffer;
    u32            Size;
};

#endif