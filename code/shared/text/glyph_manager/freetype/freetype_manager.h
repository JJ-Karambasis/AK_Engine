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

struct glyph_face {
    FT_Face      Face;
    const_buffer FontBuffer;
};

struct glyph_manager {
    arena*                 Arena;
    heap*                  Heap;
    lock_allocator*        AsyncAllocator;
    FT_Library             Library;
    async_pool<glyph_face> Faces;
};

glyph_face* FT_Glyph_Face_Get(glyph_face_id FaceID);

#endif