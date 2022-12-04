#ifndef GLYPH_STREAM_H
#define GLYPH_STREAM_H

typedef struct glyph_stream
{
    uint64_t        GlyphIndex;
    glyph_info_list Glyphs;
} glyph_stream;

glyph_info* Glyph_Stream_Consume(glyph_stream* Stream);
bool8_t     Glyph_Stream_Is_Newline(glyph_stream* Stream);
void        Glyph_Stream_Consume_Newline(glyph_stream* Stream);

#endif