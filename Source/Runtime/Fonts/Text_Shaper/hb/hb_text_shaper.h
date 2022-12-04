#ifndef HB_TEXT_SHAPER_H
#define HB_TEXT_SHAPER_H

typedef struct hb_font_t hb_font_t;
typedef struct hb_buffer_t hb_buffer_t;

typedef struct hb_text_shaper
{
    text_shaper  Base;
    arena*       Arena;
    glyph_face*  Face;
    hb_font_t*   Font;
    hb_buffer_t* Buffer;
} hb_text_shaper;

text_shaper* HB_Text_Shaper_Create(allocator* Allocator, glyph_face* Face);

#endif
