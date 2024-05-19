#ifdef TEXT_FREETYPE_FONT_MANAGER
#include <hb-ft.h>
#endif

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
        HB_SCRIPT_ARABIC,
        HB_SCRIPT_DEVANAGARI
    };
    static_assert(Array_Count(Scripts) == UBA_SCRIPT_COUNT);
    Assert(Script < UBA_SCRIPT_COUNT);
    return Scripts[Script];
}

internal hb_language_t HB_Get_Language(text_language Language) {
    local_persist const hb_language_t Languages[] = {
        HB_LANGUAGE_INVALID,
        hb_language_from_string("en", -1),
        hb_language_from_string("ar", -1)
    };
    static_assert(Array_Count(Languages) == TEXT_LANGUAGE_COUNT);
    Assert(Language < TEXT_LANGUAGE_COUNT);
    return Languages[Language];
}

text_shaper* Text_Shaper_Create(const text_shaper_create_info& CreateInfo) {
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    text_shaper* Result = Arena_Push_Struct(Arena, text_shaper);
    Result->Arena = Arena;
    AK_Mutex_Create(&Result->Mutex);
    Async_Pool_Create(&Result->FacePool, Result->Arena, CreateInfo.MaxFaceCount);

    return Result;
}

void Text_Shaper_Delete(text_shaper* Shaper) {
    if(Shaper) {
        AK_Mutex_Delete(&Shaper->Mutex);
        Allocator_Free_Memory(Core_Get_Base_Allocator(), Shaper); 
    }
}

internal text_shaper_face* Text_Shaper_Face_Get(text_shaper_face_id FaceID) {
    if(!FaceID.ID || !FaceID.Shaper) return nullptr;    
    text_shaper* Shaper = FaceID.Shaper;
    return Async_Pool_Get(&Shaper->FacePool, async_handle<text_shaper_face>(FaceID.ID));
}

text_shaper_face_id Text_Shaper_Create_Face(text_shaper* Shaper, glyph_face_id FaceID) {
    async_handle<text_shaper_face> FaceHandle = Async_Pool_Allocate(&Shaper->FacePool);
    if(FaceHandle.Is_Null()) {
        return {};
    }

    text_shaper_face* Face = Async_Pool_Get(&Shaper->FacePool, FaceHandle);

#ifdef TEXT_FREETYPE_FONT_MANAGER
    glyph_face* GlyphFace = FT_Glyph_Face_Get(FaceID);
    Face->Font = hb_ft_font_create(GlyphFace->Face, nullptr);
    hb_ft_font_set_funcs(Face->Font);
#else
#error Not Implemented
#endif

    return {
        .ID     = FaceHandle.ID,
        .Shaper = Shaper
    };
}

void Text_Shaper_Delete_Face(text_shaper* Shaper, text_shaper_face_id FaceID) {
    text_shaper_face* Face = Text_Shaper_Face_Get(FaceID);
    if(!Face) return;

    hb_face_t* HBFace = hb_font_get_face(Face->Font);

    hb_font_destroy(Face->Font);
    hb_face_destroy(HBFace);
    Face->Font = nullptr;

    Async_Pool_Free(&Shaper->FacePool, async_handle<text_shaper_face>(FaceID.ID));
}

text_shape_result Text_Shaper_Face_Shape(text_shaper_face_id FaceID, const text_shaper_shape_info& ShapeInfo) {
    text_shaper_face* Face = Text_Shaper_Face_Get(FaceID);
    if(!Face) return {};

    text_shaper* Shaper = FaceID.Shaper;
    
    AK_Mutex_Lock(&Shaper->Mutex);
    text_shaper_buffer* Buffer = Shaper->FreeBufferList;
    if(!Buffer) {
        Buffer = Arena_Push_Struct(Shaper->Arena, text_shaper_buffer);
    } else {
        SLL_Pop_Front(Shaper->FreeBufferList);
    }
    AK_Mutex_Unlock(&Shaper->Mutex);

    if(!Buffer->Buffer) {
        Buffer->Buffer = hb_buffer_create();
        if(!hb_buffer_allocation_successful(Buffer->Buffer)) return {};
    } else {
        hb_buffer_clear_contents(Buffer->Buffer);
    }    

    hb_direction_t Direction = HB_Get_Direction(ShapeInfo.Properties.Direction);
    hb_script_t Script = HB_Get_Script(ShapeInfo.Properties.Script);
    hb_language_t Language = HB_Get_Language(ShapeInfo.Properties.Language);

    hb_buffer_set_direction(Buffer->Buffer, Direction);
    hb_buffer_set_script(Buffer->Buffer, Script);
    hb_buffer_set_language(Buffer->Buffer, Language); 
    hb_buffer_add_utf8(Buffer->Buffer, ShapeInfo.Text.Str, (int)ShapeInfo.Text.Size, 0, (int)ShapeInfo.Text.Size);

    hb_shape(Face->Font, Buffer->Buffer, nullptr, 0);

    unsigned int Count;
    hb_glyph_info_t* GlyphInfo = hb_buffer_get_glyph_infos(Buffer->Buffer, &Count);
    hb_glyph_position_t* GlyphPos = hb_buffer_get_glyph_positions(Buffer->Buffer, &Count);

    text_shape_result Result = {};
    Result.GlyphCount = Count;

    if(Result.GlyphCount) {
        text_glyph_info* DstGlyphs = (text_glyph_info*)Allocator_Allocate_Memory(ShapeInfo.Allocator, Result.GlyphCount*(sizeof(text_glyph_info)+sizeof(text_shape_pos)));
        text_shape_pos*  DstPos = (text_shape_pos*)(DstGlyphs + Result.GlyphCount);

        for(u32 i = 0; i < Result.GlyphCount; i++) {
            DstGlyphs[i] = {
                .Codepoint = GlyphInfo[i].codepoint
            };

            DstPos[i] = {
                .Offset = vec2i(GlyphPos[i].x_offset >> 6, GlyphPos[i].y_offset >> 6),
                .Advance = vec2i(GlyphPos[i].x_advance >> 6, GlyphPos[i].y_advance >> 6)
            };
        }

        Result.Glyphs = DstGlyphs;
        Result.Positions = DstPos;
    }

    AK_Mutex_Lock(&Shaper->Mutex);
    SLL_Push_Front(Shaper->FreeBufferList, Buffer);
    AK_Mutex_Unlock(&Shaper->Mutex);

    return Result;
}

void Text_Shaper_Free_Result(text_shape_result* Result, allocator* Allocator) {
    if(Result && Result->GlyphCount) {
        Allocator_Free_Memory(Allocator, (void*)Result->Glyphs);
    }
}