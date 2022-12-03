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

GLYPH_FACE_SET_PIXEL_SIZE(FT_Face_Set_Pixel_Sizes)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    FT_Set_Pixel_Sizes(Face->Face, 0, PixelSize);
}

GLYPH_FACE_GENERATE_GLYPH_BITMAP(FT_Face_Generate_Glyph_Bitmap)
{
    glyph_bitmap Bitmap;
    Zero_Struct(&Bitmap, glyph_bitmap);
    
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    FT_Error Error = FT_Load_Glyph(Face->Face, Codepoint, FT_LOAD_RENDER);
    if(Error != 0) return Bitmap;
    
    FT_GlyphSlot GlyphSlot = Face->Face->glyph;
    Bitmap.Width = GlyphSlot->bitmap.width;
    Bitmap.Height = GlyphSlot->bitmap.rows;
    Bitmap.Texels = Allocate_Array(Allocator, uint32_t, Bitmap.Width*Bitmap.Height);
    
    switch(GlyphSlot->bitmap.pixel_mode)
    {
        case FT_PIXEL_MODE_GRAY:
        {
            uint8_t* DstTexels = (uint8_t*)Bitmap.Texels;
            uint8_t* SrcTexels = (uint8_t*)GlyphSlot->bitmap.buffer;
            
            for(uint32_t YIndex = 0; YIndex < Bitmap.Height; YIndex++)
            {
                for(uint32_t XIndex = 0; XIndex < Bitmap.Width; XIndex++)
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
    
    return Bitmap;
}

GLYPH_FACE_GET_CHAR_INDEX(FT_Face_Get_Char_Index)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    uint32_t Result = FT_Get_Char_Index(Face->Face, Codepoint);
    return Result;
}

GLYPH_FACE_GET_CHAR_VARIANT_INDEX(FT_Face_Get_Char_Variant_Index)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    uint32_t Result = FT_Face_GetCharVariantIndex(Face->Face, Codepoint, Variant);
    return Result;
}

GLYPH_FACE_GET_V_METRICS(FT_Face_Get_V_Metrics)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    if (Face->Face->units_per_EM != 0)
    {
        Metrics->Ascender = FT_MulFix(Face->Face->ascender, Face->Face->size->metrics.y_scale);
        Metrics->Descender = FT_MulFix(Face->Face->descender, Face->Face->size->metrics.y_scale);
        Metrics->LineGap = FT_MulFix( Face->Face->height, Face->Face->size->metrics.y_scale ) - (Metrics->Ascender - Metrics->Descender);
    }
    else
    {
        /* Bitmap-only font, eg. color bitmap font. */
        Metrics->Ascender = Face->Face->size->metrics.ascender;
        Metrics->Descender = Face->Face->size->metrics.descender;
        Metrics->LineGap = Face->Face->size->metrics.height - (Metrics->Ascender - Metrics->Descender);
    }
    
    Metrics->Ascender >>= 6;
    Metrics->Descender >>= 6;
    Metrics->LineGap >>= 6;
    
    return true;
}

GLYPH_FACE_GET_GLYPH_H_ADVANCE(FT_Face_Get_Glyph_H_Advance)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    
    FT_Fixed Advance;
    FT_Get_Advance(Face->Face, Codepoint, FT_LOAD_DEFAULT, &Advance);
    
    //TODO(JJ): Not sure why we perform this bitwise logic. Harfbuzz examples does this on their freetype 
    //advance functions
    uint32_t Result = (Advance + (1 << 9)) >> 10;
    return Result >> 6;
}

GLYPH_FACE_GET_KERNING(FT_Face_Get_Kerning)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    
    FT_Vector Kerning;
    if(FT_Get_Kerning(Face->Face, LeftCodepoint, RightCodepoint, FT_KERNING_DEFAULT, &Kerning))
        return 0;
    return Kerning.x >> 6;
}

GLYPH_FACE_GET_GLYPH_METRICS(FT_Face_Get_Glyph_Metrics)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    if(FT_Load_Glyph(Face->Face, Codepoint, FT_LOAD_BITMAP_METRICS_ONLY))
        return false;
    
    Metrics->XBearing = Face->Face->glyph->bitmap_left;
    Metrics->YBearing = Face->Face->glyph->bitmap_top;
    Metrics->Width = Face->Face->glyph->bitmap.width;
    Metrics->Height = Face->Face->glyph->bitmap.rows;
    
    return true;
}

GLYPH_FACE_GET_GLYPH_OUTLINE(FT_Face_Get_Glyph_Outline)
{
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    
    glyph_outline Result;
    Zero_Struct(&Result, glyph_outline);
    
    if(FT_Load_Glyph(Face->Face, Codepoint, FT_LOAD_DEFAULT))
        return Result;
    
    if(Face->Face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return Result;
    
    Result.Count = Face->Face->glyph->outline.n_points;
    Result.Ptr = Allocate_Array(Allocator, glyph_pt, Result.Count);
    
    for(uint64_t PointIndex = 0; PointIndex < Result.Count; PointIndex++)
    {
        int32_t x = Face->Face->glyph->outline.points[PointIndex].x;
        int32_t y = Face->Face->glyph->outline.points[PointIndex].y;
        Result.Ptr[PointIndex].x = x;
        Result.Ptr[PointIndex].y = y;
    }
    
    return Result;
}

global glyph_face_vtable G_FreeTypeFaceVTable = 
{
    FT_Face_Set_Pixel_Sizes,
    FT_Face_Generate_Glyph_Bitmap,
    FT_Face_Get_Char_Index,
    FT_Face_Get_Char_Variant_Index,
    FT_Face_Get_V_Metrics,
    FT_Face_Get_Glyph_H_Advance,
    FT_Face_Get_Kerning,
    FT_Face_Get_Glyph_Metrics,
    FT_Face_Get_Glyph_Outline
};

GLYPH_GENERATOR_CREATE_GLYPH_FACE(FT_Create_Glyph_Face)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    
    FT_Face Face;
    FT_Error Error = FT_New_Memory_Face(GlyphGenerator->Library, (const FT_Byte*)Data, (FT_Long)DataSize, 0, &Face);
    if(Error != 0) return NULL;
    
    ft_glyph_face* Result = GlyphGenerator->FreeFaces;
    if(!Result) Result = Arena_Push_Struct(GlyphGenerator->Arena, ft_glyph_face);
    else SLL_Pop_Front(GlyphGenerator->FreeFaces);
    Zero_Struct(Result, ft_glyph_face);
    
    Set_VTable(&Result->Base, &G_FreeTypeFaceVTable);
    
    Result->Face = Face;
    Result->Base.Buffer.Ptr  = Data;
    Result->Base.Buffer.Size = DataSize;
    return &Result->Base;
}

GLYPH_GENERATOR_DELETE_GLYPH_FACE(FT_Delete_Glyph_Face)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    ft_glyph_face* Face = (ft_glyph_face*)_Face;
    FT_Done_Face(Face->Face);
    SLL_Push_Front(GlyphGenerator->FreeFaces, Face);
}

GLYPH_GENERATOR_DELETE(FT_Delete)
{
    ft_glyph_generator* GlyphGenerator = (ft_glyph_generator*)_Generator;
    FT_Done_Library(GlyphGenerator->Library);
    Arena_Delete(GlyphGenerator->Arena);
}

global glyph_generator_vtable G_FreeTypeVTable = 
{
    FT_Create_Glyph_Face, 
    FT_Delete_Glyph_Face, 
    FT_Delete,
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