glyph_font* Glyph_Font_Create(allocator* Allocator, glyph_generator* Generator, buffer FontBuffer)
{
    arena* Arena     = Arena_Create(Allocator, Kilo(64));
    glyph_font* Font = Arena_Push_Struct(Arena, glyph_font);
    Font->Arena      = Arena;
    Font->Generator  = Generator;
    Font->Face       = Glyph_Generator_Create_Glyph_Face(Generator, FontBuffer.Ptr, FontBuffer.Size);
    Font->Shaper     = HB_Text_Shaper_Create(Get_Base_Allocator(Arena), Font->Face);
    return Font;
}

void Glyph_Font_Delete(glyph_font* Font)
{
    if(Font)
    {
        Text_Shaper_Delete(Font->Shaper);
        Glyph_Generator_Delete_Glyph_Face(Font->Generator, Font->Face);
        Arena_Delete(Font->Arena);
    }
}