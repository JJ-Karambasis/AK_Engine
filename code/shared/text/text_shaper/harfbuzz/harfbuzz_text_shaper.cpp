internal hb_direction_t HB_Get_Direction(uba_direction Direction) {
    local_persist const hb_direction_t Directions[] = {
        HB_DIRECTION_INVALID,
        HB_DIRECTION_LTR,
        HB_DIRECTION_RTL
    };
    static_assert(Array_Count(Directions) == UBA_DIRECTION_COUNT);
    Assert(Direction < UBA_DIRECTION_COUNT);
    return Directions[Direction];
}

internal hb_script_t HB_Get_Script(uba_script Script) {
    local_persist const hb_script_t Scripts[] = {
        HB_SCRIPT_UNKNOWN,
        HB_SCRIPT_LATIN,
    };
    static_assert(Array_Count(Scripts) == UBA_SCRIPT_COUNT);
    Assert(Script < UBA_SCRIPT_COUNT);
    return Scripts[Script];
}

internal hb_language_t HB_Get_Language(text_language Language) {
    local_persist const hb_language_t Languages[] = {
        HB_LANGUAGE_INVALID,
        hb_language_from_string("en", -1)
    };
    static_assert(Array_Count(Languages) == TEXT_LANGUAGE_COUNT);
    Assert(Language < TEXT_LANGUAGE_COUNT);
    return Languages[Language];
}

internal hb_bool_t HB_Get_Nominal_Glyph(hb_font_t* Font, void* FontData, hb_codepoint_t Codepoint, hb_codepoint_t* Glyph, void* UserData) {
    text_shaper_face* Face = (text_shaper_face*)FontData; 
    glyph_face* GlyphFace  = Face->GlyphFace;

    u32 GlyphIndex = Glyph_Face_Get_Glyph_Index(GlyphFace, Codepoint);
    if(GlyphIndex == GLYPH_INVALID) {
        return false;
    }

    *Glyph = GlyphIndex;
    return true;
}

internal unsigned int HB_Get_Nominal_Glyphs(hb_font_t* Font, void* FontData, unsigned int CodepointCount, 
                                            const hb_codepoint_t* FirstCodepoint, unsigned int CodepointStride, 
                                            hb_codepoint_t* FirstGlyph, unsigned int GlyphStride, void* UserData) {
    text_shaper_face* Face = (text_shaper_face*)FontData; 
    glyph_face* GlyphFace  = Face->GlyphFace;

    unsigned int Result = 0;
    for(; Result < CodepointCount; Result++) {
        u32 GlyphIndex = Glyph_Face_Get_Glyph_Index(GlyphFace, *FirstCodepoint);
        if(GlyphIndex == GLYPH_INVALID) {
            break;
        }
        *FirstGlyph = GlyphIndex;

        FirstCodepoint = (const hb_codepoint_t*)(((u8*)FirstCodepoint)+CodepointStride);
        FirstGlyph = (hb_codepoint_t*)(((u8*)FirstGlyph)+GlyphStride);
    }

    return Result;
}

internal hb_bool_t HB_Get_Variation_Glyph(hb_font_t* Font, void* FontData, hb_codepoint_t Codepoint, 
                                          hb_codepoint_t Variation, hb_codepoint_t* Glyph, void* UserData) {
    text_shaper_face* Face = (text_shaper_face*)FontData;
    glyph_face* GlyphFace = Face->GlyphFace;

    u32 GlyphIndex = Glyph_Face_Get_Glyph_Variant_Index(GlyphFace, Codepoint, Variation);
    if(GlyphIndex == GLYPH_INVALID) {
        return false;
    }

    *Glyph = GlyphIndex;
    return true;

}

text_shaper* Text_Shaper_Create(const text_shaper_create_info& CreateInfo) {
    uptr AllocationSize = sizeof(text_shaper);
    AllocationSize += CreateInfo.FaceCount*(sizeof(text_shaper_face)+(u32)+sizeof(ak_slot64));
    AllocationSize += CreateInfo.BufferCount*(sizeof(text_shaper_buffer)+(u32)+sizeof(ak_slot64));

    text_shaper* Result = (text_shaper*)Allocator_Allocate_Memory(Core_Get_Base_Allocator(), AllocationSize);
    Result->Faces = (text_shaper_face*)(Result+1);
    u32* FaceIndicesPtr = (u32*)(Result->Faces + CreateInfo.FaceCount);
    ak_slot64* FaceSlotsPtr = (ak_slot64*)(FaceIndicesPtr + CreateInfo.FaceCount);
    AK_Async_Slot_Map64_Init_Raw(&Result->FaceSlots, FaceIndicesPtr, FaceSlotsPtr, CreateInfo.FaceCount);

    Result->Buffers = (text_shaper_buffer*)(FaceSlotsPtr+CreateInfo.FaceCount);
    u32* BufferIndicesPtr = (u32*)(Result->Buffers+CreateInfo.BufferCount);
    ak_slot64* BufferSlotsPtr = (ak_slot64*)(BufferIndicesPtr+CreateInfo.BufferCount);
    AK_Async_Slot_Map64_Init_Raw(&Result->BufferSlots, BufferIndicesPtr, BufferSlotsPtr, CreateInfo.BufferCount);

    hb_font_funcs_t* FontFuncs = hb_font_funcs_create();
    hb_font_funcs_set_nominal_glyph_func(FontFuncs, HB_Get_Nominal_Glyph, nullptr, nullptr);
    hb_font_funcs_set_nominal_glyphs_func(FontFuncs, HB_Get_Nominal_Glyphs, nullptr, nullptr);
    hb_font_funcs_set_variation_glyph_func(FontFuncs, HB_Get_Variation_Glyph, nullptr, nullptr);

    hb_font_funcs_set_font_h_extents_func(FontFuncs, HB_Get_Font_H_Extents, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_advances_func(FontFuncs, HB_Get_Glyph_H_Advances, nullptr, nullptr);

    hb_font_funcs_set_glyph_v_advance_func(FontFuncs, HB_Get_Glyph_V_Advance, nullptr, nullptr);
    hb_font_funcs_set_glyph_v_origin_func(FontFuncs, HB_Get_Glyph_V_Origin, nullptr, nullptr);

    hb_font_funcs_set_glyph_h_kerning_func(FontFuncs, HB_Get_Glyph_H_Kerning, nullptr, nullptr);

    hb_font_funcs_set_glyph_extents_func(FontFuncs, HB_Get_Glyph_Extents, nullptr, nullptr);
    hb_font_funcs_set_glyph_contour_point_func(FontFuncs, HB_Get_Glyph_Contour_Point, nullptr, nullptr);
    hb_font_funcs_set_glyph_name_func(FontFuncs, HB_Get_Glyph_Name, nullptr, nullptr);
    hb_font_funcs_set_glyph_from_name_func(FontFuncs, HB_Get_Glyph_From_Name, nullptr, nullptr);

    hb_font_funcs_make_immutable(FontFuncs); 

    Result->FontFuncs = FontFuncs;

    return Result;
}

void Text_Shaper_Delete(text_shaper* Shaper) {
    if(Shaper) {
        Allocator_Free_Memory(Core_Get_Base_Allocator(), Shaper); 
    }
}

text_shaper_buffer_id Text_Shaper_Create_Buffer(text_shaper* Shaper) {
    hb_buffer_t* Buffer = hb_buffer_create();
    if(!hb_buffer_allocation_successful(Buffer)) return 0;

    ak_slot64 BufferSlot = AK_Async_Slot_Map64_Alloc_Slot(&Shaper->BufferSlots);
    if(!BufferSlot) {
        hb_buffer_destroy(Buffer);
        return 0;
    }

    text_shaper_buffer* Result = Shaper->Buffers + AK_Slot64_Index(BufferSlot);
    Result->Buffer = Buffer;

    return (text_shaper_buffer_id)BufferSlot;
}

void Text_Shaper_Delete_Buffer(text_shaper* Shaper, text_shaper_buffer_id BufferID) {
    ak_slot64 Slot = (ak_slot64)BufferID;
    if(AK_Async_Slot_Map64_Is_Allocated(&Shaper->BufferSlots, Slot)) {
        text_shaper_buffer* Buffer = Shaper->Buffers + AK_Slot64_Index(Slot);
        hb_buffer_destroy(Buffer->Buffer);
        Buffer->Buffer = nullptr;
        AK_Async_Slot_Map64_Free_Slot(&Shaper->BufferSlots, Slot);
    }
}

void Text_Shaper_Buffer_Add(text_shaper* Shaper, text_shaper_buffer_id BufferID, string String) {
    ak_slot64 Slot = (ak_slot64)BufferID;
    if(AK_Async_Slot_Map64_Is_Allocated(&Shaper->BufferSlots, Slot)) {
        text_shaper_buffer* Buffer = Shaper->Buffers + AK_Slot64_Index(Slot);
        hb_buffer_add_utf8(Buffer->Buffer, String.Str, (int)String.Size, Buffer->CurrentOffset, (int)String.Size);
        Buffer->CurrentOffset += Safe_U32(String.Size);
    }
}

void Text_Shaper_Buffer_Clear(text_shaper* Shaper, text_shaper_buffer_id BufferID) {
    ak_slot64 Slot = (ak_slot64)BufferID;
    if(AK_Async_Slot_Map64_Is_Allocated(&Shaper->BufferSlots, Slot)) {
        text_shaper_buffer* Buffer = Shaper->Buffers + AK_Slot64_Index(Slot);
        hb_buffer_clear_contents(Buffer->Buffer);
        Buffer->CurrentOffset = 0;
    }
}

void Text_Shaper_Buffer_Set_Properties(text_shaper* Shaper, text_shaper_buffer_id BufferID, const text_shaper_buffer_properties& Properties) {
    ak_slot64 Slot = (ak_slot64)BufferID;
    if(AK_Async_Slot_Map64_Is_Allocated(&Shaper->BufferSlots, Slot)) {
        text_shaper_buffer* Buffer = Shaper->Buffers + AK_Slot64_Index(Slot);
        hb_direction_t Direction = HB_Get_Direction(Properties.Direction);
        hb_script_t Script = HB_Get_Script(Properties.Script);
        hb_language_t Language = HB_Get_Language(Properties.Language);

        hb_buffer_set_direction(Buffer->Buffer, Direction);
        hb_buffer_set_script(Buffer->Buffer, Script);
        hb_buffer_set_language(Buffer->Buffer, Language);        
    }
}

text_shaper_face_id Text_Shaper_Create_Face(text_shaper* Shaper, glyph_face* GlyphFace) {
    ak_slot64 FaceSlot = AK_Async_Slot_Map64_Alloc_Slot(&Shaper->FaceSlots);
    if(!FaceSlot) {
        return 0;
    }

    text_shaper_face* Face = Shaper->Faces + AK_Slot64_Index(FaceSlot);
    Face->GlyphFace = GlyphFace;
    const_buffer FontBuffer = Glyph_Face_Get_Font_Buffer(GlyphFace);
    hb_blob_t* Blob = hb_blob_create((const char*)FontBuffer.Ptr, Safe_U32(FontBuffer.Size), HB_MEMORY_MODE_READONLY, GlyphFace, nullptr);
    Face->Face = hb_face_create(Blob, 0);
    hb_blob_destroy(Blob);
    Face->Font = hb_font_create(Face->Face);
    AK_Mutex_Create(&Face->Mutex);
    hb_font_set_funcs(Face->Font, Shaper->FontFuncs, GlyphFace, nullptr);

    return (text_shaper_face_id)FaceSlot;
}

void                  Text_Shaper_Delete_Face(text_shaper_face_id Face);
text_shape_result     Text_Shaper_Face_Shape(text_shaper_face_id Face, text_shaper_buffer_id Buffer, allocator* Allocator);
void                  Text_Shaper_Free_Result(text_shape_result* Result, allocator* Allocator);
