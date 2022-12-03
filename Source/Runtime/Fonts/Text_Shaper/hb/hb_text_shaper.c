#include <harfbuzz/src/hb.h>

typedef struct hb_context
{
    heap*            Heap;
    hb_font_funcs_t* FontFuncs;
} hb_context;

static hb_context* G__HB;

void* HB_Heap_Alloc(size_t Size)
{
    size_t AllocationSize = Size+sizeof(size_t);
    size_t* NewMemory = (size_t*)Heap_Allocate(G__HB->Heap, AllocationSize, MEMORY_NO_CLEAR);
    *NewMemory = Size;
    return NewMemory+1;
}

void HB_Heap_Free(void* Ptr)
{
    if(Ptr) 
    {
        size_t* NewPtr = (size_t*)Ptr;
        Heap_Free(G__HB->Heap, &NewPtr[-1]);
    }
}

void* HB_Heap_Calloc(size_t Count, size_t Size)
{
    size_t AllocationSize = (Size*Count)+sizeof(size_t);
    size_t* NewMemory = (size_t*)Heap_Allocate(G__HB->Heap, AllocationSize, MEMORY_NO_CLEAR);
    *NewMemory = Size*Count;
    return NewMemory+1;
}

void* HB_Heap_Realloc(void* Ptr, size_t Size)
{
    size_t AllocationSize = Size+sizeof(size_t);
    
    size_t* NewMemory = (size_t*)Heap_Allocate(G__HB->Heap, AllocationSize, MEMORY_NO_CLEAR);
    if(Ptr)
    {
        size_t CurrentSize = ((size_t*)Ptr)[-1];
        Memory_Copy(NewMemory+1, Ptr, Min(CurrentSize, Size));
        HB_Heap_Free(Ptr);
    }
    
    *NewMemory = Size;
    return NewMemory+1;
}

static hb_bool_t HB_Get_Nominal_Glyph_Func(hb_font_t* Font, void* FontData, hb_codepoint_t Unicode, hb_codepoint_t* Glyph,
                                           void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    uint32_t CharIndex = Glyph_Face_Get_Char_Index(Face, Unicode);
    if(!CharIndex) return false;
    *Glyph = CharIndex;
    return true;
}

static uint32_t HB_Get_Nominal_Glyphs_Func(hb_font_t* Font, void* FontData, unsigned int Count, const hb_codepoint_t *FirstUnicode,
                                           unsigned int UnicodeStride, hb_codepoint_t* FirstGlyph, unsigned int GlyphStride,
                                           void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    
    uint32_t Result = 0;
    
    hb_codepoint_t* Glyph = FirstGlyph;
    const hb_codepoint_t* Code = FirstUnicode;
    for(unsigned int Index = 0; Index < Count; Index++)
    {
        uint32_t CharIndex = Glyph_Face_Get_Char_Index(Face, *Code);
        if(CharIndex)
        {
            Result++;
            *Glyph = CharIndex;
            Code = (const hb_codepoint_t*)(((const uint8_t*)Code)+UnicodeStride);
            Glyph = (hb_codepoint_t*)(((uint8_t*)Glyph)+GlyphStride);
        }
    }
    
    return Result;
}

static hb_bool_t HB_Get_Variation_Glyph_Func(hb_font_t* Font, void* FontData, hb_codepoint_t Unicode, hb_codepoint_t Variation,
                                             hb_codepoint_t* Glyph, void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    uint32_t CharIndex = Glyph_Face_Get_Char_Variant_Index(Face, Unicode, Variation);
    if(!CharIndex) return false;
    *Glyph = CharIndex;
    return true;
}

static hb_bool_t HB_Get_Font_H_Extents_Func(hb_font_t* Font, void* FontData, hb_font_extents_t* Metrics, void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    
    glyph_face_v_metrics VMetrics;
    if(!Glyph_Face_Get_V_Metrics(Face, &VMetrics))
        return false;
    
    Metrics->ascender = VMetrics.Ascender;
    Metrics->descender = VMetrics.Descender;
    Metrics->line_gap = VMetrics.LineGap;
    return true;
}

static void HB_Get_Glyph_H_Advances_Func(hb_font_t* Font, void* FontData, unsigned int Count, const hb_codepoint_t* FirstGlyph, 
                                         unsigned int GlyphStride, hb_position_t* FirstAdvance, unsigned int AdvanceStride,
                                         void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    
    for(unsigned int Index = 0; Index < Count; Index++)
    {
        hb_codepoint_t Glyph = *FirstGlyph;
        *FirstAdvance = Glyph_Face_Get_Glyph_H_Advance(Face, Glyph);
        FirstGlyph = (const hb_codepoint_t*)(((const uint8_t*)FirstGlyph)+GlyphStride);
        FirstAdvance = (hb_position_t*)(((const uint8_t*)FirstAdvance)+AdvanceStride);
    }
}

static hb_position_t HB_Get_Glyph_H_Kerning(hb_font_t* Font, void* FontData, hb_codepoint_t LeftGlyph, hb_codepoint_t RightGlyph, 
                                            void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    hb_position_t Kerning = Glyph_Face_Get_Kerning(Face, LeftGlyph, RightGlyph);
    return Kerning;
}

static hb_bool_t HB_Get_Glyph_Extents_Func(hb_font_t* Font, void* FontData, hb_codepoint_t Glyph, hb_glyph_extents_t* Extents,
                                           void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    
    glyph_metrics Metrics;
    if(!Glyph_Face_Get_Glyph_Metrics(Face, Glyph, &Metrics))
        return false;
    
    Extents->x_bearing = Metrics.XBearing;
    Extents->y_bearing = Metrics.YBearing;
    Extents->width  = Metrics.Width;
    Extents->height = Metrics.Height;
    
    return true;
}

static hb_bool_t HB_Get_Glyph_Contour_Point_Func(hb_font_t* Font, void* FontData, hb_codepoint_t Glyph, unsigned int PointIndex, 
                                                 hb_position_t* x, hb_position_t* y, void* UserData)
{
    glyph_face* Face = (glyph_face*)FontData;
    
    glyph_outline Outline = Glyph_Face_Get_Glyph_Outline(Face, Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), Glyph);
    if(!Outline.Ptr) return false;
    if(PointIndex >= Outline.Count) return false;
    
    *x = Outline.Ptr[PointIndex].x;
    *y = Outline.Ptr[PointIndex].y;
    
    return true;
}

static hb_bool_t HB_Get_Glyph_Name_Func(hb_font_t* Font, void* FontData, hb_codepoint_t Glyph, char* Name, unsigned int Size, void* UserData)
{
    //TODO(JJ): Impelement
    return false;
}

static hb_bool_t HB_Get_Glyph_From_Name_Func(hb_font_t* Font, void* FontData, const char* Name, int Length, hb_codepoint_t* Glyph, void* UserData)
{
    //TODO(JJ): Impelement
    *Glyph = 0;
    return false;
}

TEXT_SHAPER_SHAPE(HB_Text_Shaper_Shape)
{
    shape_list Result;
    Zero_Struct(&Result, shape_list);
    
    hb_text_shaper* Shaper = (hb_text_shaper*)_Shaper;
    
    hb_buffer_t* Buffer = hb_buffer_create();
    hb_buffer_set_flags(Buffer, HB_BUFFER_FLAG_BOT|HB_BUFFER_FLAG_EOT);
    
    int32_t TextLength = Safe_U64_S32(Text.Length);
    hb_buffer_add_utf8(Buffer, (const char*)Text.Str, TextLength, 0, TextLength);
    
    hb_buffer_set_direction(Buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(Buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(Buffer, hb_language_from_string("en", -1));
    
    hb_shape(Shaper->Font, Buffer, NULL, 0);
    
    uint32_t GlyphCount;
    hb_glyph_info_t* GlyphInfos = hb_buffer_get_glyph_infos(Buffer, &GlyphCount);
    hb_glyph_position_t* GlyphPositions = hb_buffer_get_glyph_positions(Buffer, &GlyphCount);
    
    if(GlyphCount)
    {
        Result.Count = GlyphCount;
        Result.Ptr = Allocate_Array(Allocator, shape, Result.Count);
        
        for(uint32_t GlyphIndex = 0; GlyphIndex < GlyphCount; GlyphIndex++)
        {
            hb_glyph_info_t* GlyphInfo = GlyphInfos + GlyphIndex;
            hb_glyph_position_t* GlyphPosition = GlyphPositions + GlyphIndex;
            
            shape Shape;
            Shape.Codepoint = GlyphInfo->codepoint;
            Shape.XOffset = GlyphPosition->x_offset;
            Shape.YOffset = GlyphPosition->y_offset;
            Shape.XAdvance = GlyphPosition->x_advance;
            Shape.YAdvance = GlyphPosition->y_advance;
            
            Result.Ptr[GlyphIndex] = Shape;
        }
    }
    
    hb_buffer_destroy(Buffer);
    
    return Result;
}

TEXT_SHAPER_DELETE(HB_Text_Shaper_Delete)
{
    hb_text_shaper* Shaper = (hb_text_shaper*)_Shaper;
    hb_font_destroy(Shaper->Font);
    Arena_Delete(Shaper->Arena);
}

global text_shaper_vtable G_HBShaperVTable = 
{
    HB_Text_Shaper_Shape,
    HB_Text_Shaper_Delete
};

text_shaper* HB_Text_Shaper_Create(allocator* Allocator, glyph_face* Face)
{
    if(!G__HB)
    {
        heap* Heap = Heap_Create(Core_Get_Thread_Context()->MainAllocator, Kilo(512));
        G__HB = Heap_Allocate_Struct(Heap, hb_context);
        G__HB->Heap = Heap;
        
        hb_font_funcs_t* FontFuncs = hb_font_funcs_create();
        
        hb_font_funcs_set_nominal_glyph_func(FontFuncs, HB_Get_Nominal_Glyph_Func, NULL, NULL);
        hb_font_funcs_set_nominal_glyphs_func(FontFuncs, HB_Get_Nominal_Glyphs_Func, NULL, NULL);
        hb_font_funcs_set_variation_glyph_func(FontFuncs, HB_Get_Variation_Glyph_Func, NULL, NULL);
        hb_font_funcs_set_font_h_extents_func(FontFuncs, HB_Get_Font_H_Extents_Func, NULL, NULL);
        hb_font_funcs_set_glyph_h_advances_func(FontFuncs, HB_Get_Glyph_H_Advances_Func, NULL, NULL);
        hb_font_funcs_set_glyph_h_kerning_func(FontFuncs, HB_Get_Glyph_H_Kerning, NULL, NULL);
        hb_font_funcs_set_glyph_extents_func(FontFuncs, HB_Get_Glyph_Extents_Func, NULL, NULL);
        hb_font_funcs_set_glyph_contour_point_func (FontFuncs, HB_Get_Glyph_Contour_Point_Func, NULL, NULL);
        hb_font_funcs_set_glyph_name_func (FontFuncs, HB_Get_Glyph_Name_Func, NULL, NULL);
        hb_font_funcs_set_glyph_from_name_func (FontFuncs, HB_Get_Glyph_From_Name_Func, NULL, NULL);
        
        hb_font_funcs_make_immutable(FontFuncs);
        G__HB->FontFuncs = FontFuncs;
    }
    
    arena* Arena = Arena_Create(Allocator, Kilo(32));
    hb_text_shaper* Shaper = Arena_Push_Struct(Arena, hb_text_shaper);
    Shaper->Arena = Arena;
    
    const_buffer Buffer = Face->Buffer;
    hb_blob_t* Blob = hb_blob_create((const char*)Buffer.Ptr, Safe_U64_U32(Buffer.Size), HB_MEMORY_MODE_READONLY, 
                                     Face, NULL);
    hb_face_t* HBFace = hb_face_create(Blob, 0);
    hb_blob_destroy(Blob);
    
    hb_font_t* Font = hb_font_create(HBFace);
    hb_face_destroy(HBFace);
    
    hb_font_set_funcs(Font, G__HB->FontFuncs, Face, NULL);
    
    Shaper->Font = Font;
    Set_VTable(&Shaper->Base, &G_HBShaperVTable);
    return &Shaper->Base;
}