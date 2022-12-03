#ifndef HB_TEXT_SHAPER_H
#define HB_TEXT_SHAPER_H

typedef struct hb_font_t hb_font_t;

typedef struct hb_text_shaper
{
    text_shaper Base;
    arena*      Arena;
    hb_font_t*  Font;
} hb_text_shaper;

text_shaper* HB_Text_Shaper_Create(allocator* Allocator, glyph_face* Face);

#endif
