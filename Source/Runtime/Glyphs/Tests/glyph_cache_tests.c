typedef struct GlyphCacheTest
{
    custom_allocator* Allocator;
    glyph_generator*  Generator;
} GlyphCacheTest;

glyph_generator* Stub_Glyph_Generator_Create(allocator* Allocator)
{
    return NULL;
}

void Stub_Glyph_Generator_Delete(glyph_generator* Generator, allocator* Allocator)
{
    
}

UTEST_F_SETUP(GlyphCacheTest)
{
    custom_allocator* Allocator = Allocate_Custom_Allocator();
    utest_fixture->Allocator = Allocator;
    utest_fixture->Generator = Stub_Glyph_Generator_Create(Get_Base_Allocator(Allocator));
}

UTEST_F(GlyphCacheTest, Test_Cache)
{
#if 0 
    glyph_cache* Cache = Glyph_Cache_Create(Get_Base_Allocator(utest_fixture->Allocator), utest_fixture->Generator, 4);
    glyph* Glyph3 = Glyph_Cache_Get(Cache, (font_face*)0, 32);
    ASSERT_EQ(Cache->LRU.Last->Glyph.Codepoint, 32);
    
    glyph* Glyph = Glyph_Cache_Get(Cache, (font_face*)0, 33);
    ASSERT_EQ(Cache->LRU.Last->Glyph.Codepoint, 33);
    
    Glyph_Cache_Get(Cache, (font_face*)0, 34);
    ASSERT_EQ(Cache->LRU.Last->Glyph.Codepoint, 34);
    
    glyph* Glyph2 = Glyph_Cache_Get(Cache, (font_face*)0, 33);
    ASSERT_EQ(Cache->LRU.Last->Glyph.Codepoint, 33);
    ASSERT_EQ(Cache->LRU.Last->PrevLRUEntry->Glyph.Codepoint, 34);
    ASSERT_EQ(Glyph, Glyph2);
    
    Glyph_Cache_Get(Cache, (font_face*)0, 35);
    Glyph_Cache_Get(Cache, (font_face*)0, 36);
    ASSERT_EQ(Glyph3->Codepoint, 36);
    ASSERT_EQ(Cache->LRU.First->Glyph.Codepoint, 34);
    ASSERT_EQ(Cache->LRU.Last->Glyph.Codepoint, 36);
    ASSERT_EQ(Cache->LRU.First->NextLRUEntry->Glyph.Codepoint, 33);
    ASSERT_EQ(Cache->LRU.Last->PrevLRUEntry->Glyph.Codepoint, 35);
    
    Glyph_Cache_Delete(Cache);
#endif
}

UTEST_F_TEARDOWN(GlyphCacheTest)
{
    custom_allocator* Allocator = utest_fixture->Allocator;
    Stub_Glyph_Generator_Delete(utest_fixture->Generator, Get_Base_Allocator(Allocator));
    
    Test_Custom_Allocator_Is_Not_Leaking(utest_result, Allocator);
    Free_Custom_Allocator(Allocator);
}