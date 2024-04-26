#ifndef HARFBUZZ_TEXT_SHAPER_H
#define HARFBUZZ_TEXT_SHAPER_H

#include <hb.h>

struct text_shaper_face {
    hb_face_t*  Face;
    hb_font_t*  Font;
    glyph_face* GlyphFace;
};

struct text_shaper_buffer {
    hb_buffer_t* Buffer;
    u32          CurrentOffset;
};

struct text_shaper {
    ak_async_slot_map64 FaceSlots;
    text_shaper_face*   Faces;
    ak_async_slot_map64 BufferSlots;
    text_shaper_buffer* Buffers;
    hb_font_funcs_t*    FontFuncs;
};

#endif