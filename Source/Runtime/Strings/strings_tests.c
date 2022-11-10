UTEST(Strings, String_Encoding)
{
    custom_allocator* Allocator = Allocate_Custom_Allocator();
    arena* StringArena = Arena_Create(Get_Base_Allocator(Allocator), Mega(1));
    
#if 0
    buffer Buffer = Read_Entire_File_Null_Term(StringArena, "test_data/quickbrown.txt");
    
    str8 StrUTF8 = Str8(Buffer.Data, Buffer.Length);
    utf8_stream_reader ReaderUTF8 = UTF8_Stream_Reader(StrUTF8);
    
    str16 StrUTF16 = UTF8_To_UTF16(StringArena, StrUTF8);
    utf16_stream_reader ReaderUTF16 = UTF16_Stream_Reader(StrUTF16);
    
    str32 StrUTF32 = UTF8_To_UTF32(StringArena, StrUTF8);
    strc StrC = UTF8_To_Ascii(StringArena, StrUTF8);
    ASSERT_EQ(StrC.Length, StrUTF32.Length);
    for(size_t i = 0; i < StrUTF32.Length; i++)
    {
        uint32_t Codepoint8 = ReaderUTF8.Consume();
        uint32_t Codepoint16 = ReaderUTF16.Consume();
        ASSERT_EQ(StrUTF32[i], Codepoint8);
        ASSERT_EQ(StrUTF32[i], Codepoint16);
        
        if(StrUTF32[i] <= 127)
            ASSERT_EQ((char)StrUTF32[i], StrC[i]);
        else
            ASSERT_EQ(StrC[i], '?');
    }
    ASSERT_FALSE(ReaderUTF16.Is_Valid());
    ASSERT_FALSE(ReaderUTF8.Is_Valid());
    
    str8 StrUTF8_2 = UTF16_To_UTF8(StringArena, StrUTF16);
    ASSERT_EQ(StrUTF8.Length, StrUTF8_2.Length);
    for(size_t i = 0; i < StrUTF8.Length; i++)
        ASSERT_EQ(StrUTF8[i], StrUTF8_2[i]);
    
    str32 StrUTF32_2 = UTF16_To_UTF32(StringArena, StrUTF16);
    ASSERT_EQ(StrUTF32.Length, StrUTF32_2.Length);
    for(size_t i = 0; i < StrUTF32.Length; i++)
        ASSERT_EQ(StrUTF32[i], StrUTF32_2[i]);
    
    strc StrC_2 = UTF16_To_Ascii(StringArena, StrUTF16);
    ASSERT_EQ(StrC.Length, StrC_2.Length);
    for(size_t i = 0; i < StrC.Length; i++)
        ASSERT_EQ(StrC[i], StrC_2[i]);
    
    str8 StrUTF8_3 = UTF32_To_UTF8(StringArena, StrUTF32);
    ASSERT_EQ(StrUTF8.Length, StrUTF8_3.Length);
    for(size_t i = 0; i < StrUTF8.Length; i++)
        ASSERT_EQ(StrUTF8[i], StrUTF8_3[i]);
    
    str16 StrUTF16_2 = UTF32_To_UTF16(StringArena, StrUTF32);
    ASSERT_EQ(StrUTF16.Length, StrUTF16_2.Length);
    for(size_t i = 0; i < StrUTF16.Length; i++)
        ASSERT_EQ(StrUTF16[i], StrUTF16_2[i]);
    
    strc StrC_3 = UTF32_To_Ascii(StringArena, StrUTF32);
    ASSERT_EQ(StrC.Length, StrC_3.Length);
    for(size_t i = 0; i < StrC.Length; i++)
        ASSERT_EQ(StrC[i], StrC_3[i]);
#endif
    Arena_Delete(StringArena);
    Test_Custom_Allocator_Is_Not_Leaking(utest_result, Allocator);
    Free_Custom_Allocator(Allocator);
}