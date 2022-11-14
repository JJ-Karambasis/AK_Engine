void* FT_Alloc(FT_Memory Memory, long Size)
{
    heap* Heap = (heap*)Memory->user;
    return Heap_Allocate(Heap, Size, MEMORY_NO_CLEAR);
}

void FT_Free(FT_Memory Memory, void* Block)
{
    heap* Heap = (heap*)Memory->user;
    Heap_Free(Heap, Block);
}

void* FT_Realloc(FT_Memory Memory, long CurrentSize, long NewSize, void* Block)
{
    if(CurrentSize == NewSize) return Block;
    heap* Heap = (heap*)Memory->user;
    void* NewMemory = Heap_Allocate(Heap, NewSize, MEMORY_NO_CLEAR);
    Memory_Copy(NewMemory, Block, Min(CurrentSize, NewSize));
    if(Block) Heap_Free(Heap, Block);
    return NewMemory;
}

GLYPH_GENERATOR_CREATE_FONT_FACE(FT_Create_Font_Face)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    
    FT_Face Face;
    FT_Error Error = FT_New_Memory_Face(GlyphGenerator->Library, (const FT_Byte*)Data, (FT_Long)DataSize, 0, &Face);
    if(Error != 0) return NULL;
    
    Error = FT_Set_Pixel_Sizes(Face, 0, FaceHeight);
    if(Error != 0) return NULL;
    
    ft_font_face* Result = GlyphGenerator->FreeFaces;
    if(!Result) Result = Arena_Push_Struct(GlyphGenerator->Arena, ft_font_face);
    else SLL_Pop_Front(GlyphGenerator->FreeFaces);
    Zero_Struct(Result, ft_font_face);
    
    Result->FTFace = Face;
    Result->Face.Height = FaceHeight;
    return &Result->Face;
}

GLYPH_GENERATOR_DELETE_FONT_FACE(FT_Delete_Font_Face)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    ft_font_face* Face = (ft_font_face*)_Face;
    FT_Done_Face(Face->FTFace);
    SLL_Push_Front(GlyphGenerator->FreeFaces, Face);
}

GLYPH_GENERATOR_DELETE(FT_Delete)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    FT_Done_Library(GlyphGenerator->Library);
    Arena_Delete(GlyphGenerator->Arena);
}

GLYPH_GENERATOR_GENERATE_GLYPH_METRICS(FT_Generate_Glyph_Metrics)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    ft_font_face*       FontFace = (ft_font_face*)_Face;
    
    FT_Face Face = FontFace->FTFace;
    FT_Error Error = FT_Load_Char(Face, Codepoint, FT_LOAD_BITMAP_METRICS_ONLY);
    if(Error != 0) return false;
    
    Glyph->Codepoint = Codepoint;
    FT_GlyphSlot GlyphSlot = Face->glyph;
    Glyph->Width = GlyphSlot->bitmap.width;
    Glyph->Height = GlyphSlot->bitmap.rows;
    
    return true;
}

GLYPH_GENERATOR_GENERATE_GLYPH_BITMAP(FT_Generate_Glyph_Bitmap)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    ft_font_face*       FontFace = (ft_font_face*)_Face;
    
    FT_Face Face = FontFace->FTFace;
    FT_Error Error = FT_Load_Char(Face, Codepoint, FT_LOAD_RENDER);
    if(Error != 0) return false;
    
    FT_GlyphSlot GlyphSlot = Face->glyph;
    Bitmap->Width = GlyphSlot->bitmap.width;
    Bitmap->Height = GlyphSlot->bitmap.rows;
    Bitmap->Texels = Allocate_Array(Allocator, uint32_t, Bitmap->Width*Bitmap->Height);
    
    switch(GlyphSlot->bitmap.pixel_mode)
    {
        case FT_PIXEL_MODE_GRAY:
        {
            uint8_t* DstTexels = (uint8_t*)Bitmap->Texels;
            uint8_t* SrcTexels = (uint8_t*)GlyphSlot->bitmap.buffer;
            
            for(uint32_t YIndex = 0; YIndex < Bitmap->Height; YIndex++)
            {
                for(uint32_t XIndex = 0; XIndex < Bitmap->Width; XIndex++)
                {
                    *DstTexels++ = *SrcTexels;
                    *DstTexels++ = *SrcTexels;
                    *DstTexels++ = *SrcTexels;
                    *DstTexels++ = *SrcTexels;
                    SrcTexels++;
                }
            }
        } break;
        
        Invalid_Default_Case;
    }
    
    return true;
}

global glyph_generator_vtable G_FreeTypeVTable = 
{
    FT_Create_Font_Face, 
    FT_Delete_Font_Face, 
    FT_Delete,
    FT_Generate_Glyph_Metrics,
    FT_Generate_Glyph_Bitmap
};

glyph_generator* FT_Glyph_Generator_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(2));
    ft_glyph_generator* Generator = Arena_Push_Struct(Arena, ft_glyph_generator);
    Set_VTable(&Generator->Generator, &G_FreeTypeVTable);
    Generator->Arena = Arena;
    Generator->FreeTypeHeap = Heap_Create(Get_Base_Allocator(Generator->Arena), Kilo(512));
    
    FT_Memory Memory = Arena_Push_Struct(Generator->Arena, struct FT_MemoryRec_);
    Memory->alloc   = FT_Alloc;
    Memory->free    = FT_Free;
    Memory->realloc = FT_Realloc;
    Memory->user    = Generator->FreeTypeHeap;
    
    FT_Library Library;
    FT_Error Error = FT_New_Library(Memory, &Library);
    if(Error != 0) return NULL;
    
    FT_Add_Default_Modules(Library);
    FT_Set_Default_Properties(Library);
    Generator->Library = Library;
    return &Generator->Generator;
}
