glyph_info* Glyph_Stream_Consume(glyph_stream* Stream)
{
    if(Stream->GlyphIndex >= Stream->Glyphs.Count) return NULL;
    glyph_info* Result = Stream->Glyphs.Ptr + Stream->GlyphIndex++;
    return Result;
}

bool8_t Glyph_Stream_Is_Newline(glyph_stream* Stream)
{
    if(Stream->GlyphIndex < Stream->Glyphs.Count)
    {
        glyph_info* GlyphInfo = Stream->Glyphs.Ptr + Stream->GlyphIndex;
        return GlyphInfo->Glyph->Codepoint == '\n' || GlyphInfo->Glyph->Codepoint == '\r';
    }
    return false;
}

void Glyph_Stream_Consume_Newline(glyph_stream* Stream)
{
    Assert(Stream->GlyphIndex < Stream->Glyphs.Count);
    
    glyph_info* GlyphInfo = Stream->Glyphs.Ptr + Stream->GlyphIndex;
    if(GlyphInfo->Glyph->Codepoint == '\n')
    {
        Stream->GlyphIndex++;
        if(Stream->GlyphIndex < Stream->Glyphs.Count)
        {
            GlyphInfo = Stream->Glyphs.Ptr + Stream->GlyphIndex;
            if(GlyphInfo->Glyph->Codepoint == '\r')
                Stream->GlyphIndex++;
        }
    } 
    else if(GlyphInfo->Glyph->Codepoint == '\r')
    {
        Stream->GlyphIndex++;
        if(Stream->GlyphIndex < Stream->Glyphs.Count)
        {
            GlyphInfo = Stream->Glyphs.Ptr + Stream->GlyphIndex;
            if(GlyphInfo->Glyph->Codepoint == '\n')
                Stream->GlyphIndex++;
        }
    }
    else
    {
        Invalid_Code();
    }
}
