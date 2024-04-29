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

    return Result;
}

void Text_Shaper_Delete(text_shaper* Shaper) {
    if(Shaper) {
        Allocator_Free_Memory(Core_Get_Base_Allocator(), Shaper); 
    }
}

internal text_shaper_buffer* Text_Shaper_Buffer_Get(text_shaper_buffer_id BufferID) {
    if(!BufferID.Generation || !BufferID.Buffer) return nullptr;
    return AK_Async_Slot_Map64_Is_Allocated(&BufferID.Buffer->Shaper->BufferSlots, (ak_slot64)BufferID.Generation) ? BufferID.Buffer : nullptr;
}

text_shaper_buffer_id Text_Shaper_Create_Buffer(text_shaper* Shaper) {
    hb_buffer_t* Buffer = hb_buffer_create();
    if(!hb_buffer_allocation_successful(Buffer)) return {};

    ak_slot64 BufferSlot = AK_Async_Slot_Map64_Alloc_Slot(&Shaper->BufferSlots);
    if(!BufferSlot) {
        hb_buffer_destroy(Buffer);
        return {};
    }

    text_shaper_buffer* Result = Shaper->Buffers + AK_Slot64_Index(BufferSlot);
    Result->Buffer = Buffer;
    Result->Shaper = Shaper;
    Result->CurrentOffset = 0;

    return {
        .Generation = BufferSlot,
        .Buffer = Result
    };
}

void Text_Shaper_Delete_Buffer(text_shaper* Shaper, text_shaper_buffer_id BufferID) {
    text_shaper_buffer* Buffer = Text_Shaper_Buffer_Get(BufferID);
    if(!Buffer) return;

    hb_buffer_destroy(Buffer->Buffer);
    Buffer->Buffer = nullptr;
    AK_Async_Slot_Map64_Free_Slot(&Shaper->BufferSlots, BufferID.Generation);
}

void Text_Shaper_Buffer_Add(text_shaper_buffer_id BufferID, string String) {
    text_shaper_buffer* Buffer = Text_Shaper_Buffer_Get(BufferID);
    if(!Buffer) return;
    hb_buffer_add_utf8(Buffer->Buffer, String.Str, (int)String.Size, Buffer->CurrentOffset, (int)String.Size);
    Buffer->CurrentOffset += Safe_U32(String.Size);
}

void Text_Shaper_Buffer_Clear(text_shaper_buffer_id BufferID) {
    text_shaper_buffer* Buffer = Text_Shaper_Buffer_Get(BufferID);
    if(!Buffer) return;
    hb_buffer_clear_contents(Buffer->Buffer);
    Buffer->CurrentOffset = 0;
}

void Text_Shaper_Buffer_Set_Properties(text_shaper_buffer_id BufferID, const text_shaper_buffer_properties& Properties) {
    text_shaper_buffer* Buffer = Text_Shaper_Buffer_Get(BufferID);
    if(!Buffer) return;

    hb_direction_t Direction = HB_Get_Direction(Properties.Direction);
    hb_script_t Script = HB_Get_Script(Properties.Script);
    hb_language_t Language = HB_Get_Language(Properties.Language);

    hb_buffer_set_direction(Buffer->Buffer, Direction);
    hb_buffer_set_script(Buffer->Buffer, Script);
    hb_buffer_set_language(Buffer->Buffer, Language);        
}

internal text_shaper_face* Text_Shaper_Face_Get(text_shaper_face_id FaceID) {
    if(!FaceID.Generation || !FaceID.Face) return nullptr;
    return AK_Async_Slot_Map64_Is_Allocated(&FaceID.Face->Shaper->FaceSlots, (ak_slot64)FaceID.Generation) ? FaceID.Face : nullptr;
}

text_shaper_face_id Text_Shaper_Create_Face(text_shaper* Shaper, glyph_face_id FaceID) {
    ak_slot64 FaceSlot = AK_Async_Slot_Map64_Alloc_Slot(&Shaper->FaceSlots);
    if(!FaceSlot) {
        return {};
    }

    text_shaper_face* Face = Shaper->Faces + AK_Slot64_Index(FaceSlot);
    Face->GlyphFace = FaceID;
    const_buffer FontBuffer = Glyph_Face_Get_Font_Buffer(FaceID);
    hb_blob_t* Blob = hb_blob_create((const char*)FontBuffer.Ptr, Safe_U32(FontBuffer.Size), HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    Face->Face = hb_face_create(Blob, 0);
    hb_blob_destroy(Blob);
    Face->Font = hb_font_create(Face->Face);

    return {
        .Generation = FaceSlot,
        .Face = Face
    };
}

void Text_Shaper_Delete_Face(text_shaper* Shaper, text_shaper_face_id FaceID) {
    text_shaper_face* Face = Text_Shaper_Face_Get(FaceID);
    if(!Face) return;

    hb_face_t* HBFace = hb_font_get_face(Face->Font);

    hb_font_destroy(Face->Font);
    hb_face_destroy(HBFace);
    Face->Font = nullptr;
    AK_Async_Slot_Map64_Free_Slot(&Shaper->FaceSlots, FaceID.Generation);
}

text_shape_result Text_Shaper_Face_Shape(text_shaper_face_id FaceID, text_shaper_buffer_id BufferID, allocator* Allocator) {
    text_shaper_buffer* Buffer = Text_Shaper_Buffer_Get(BufferID);
    if(!Buffer) return {};

    text_shaper_face* Face = Text_Shaper_Face_Get(FaceID);
    if(!Face) return {};

    hb_shape(Face->Font, Buffer->Buffer, nullptr, 0);

    unsigned int Count;
    hb_glyph_info_t* GlyphInfo = hb_buffer_get_glyph_infos(Buffer->Buffer, &Count);
    hb_glyph_position_t* GlyphPos = hb_buffer_get_glyph_positions(Buffer->Buffer, &Count);

    text_shape_result Result = {};
    Result.GlyphCount = Count;

    if(Result.GlyphCount) {
        text_glyph_info* DstGlyphs = (text_glyph_info*)Allocator_Allocate_Memory(Allocator, Result.GlyphCount*(sizeof(text_glyph_info)+sizeof(text_shape_pos)));
        text_shape_pos*  DstPos = (text_shape_pos*)(DstGlyphs + Result.GlyphCount);

        for(u32 i = 0; i < Result.GlyphCount; i++) {
            DstGlyphs[i] = {
                .Codepoint = GlyphInfo[i].codepoint
            };

            DstPos[i] = {
                .XOffset = GlyphPos[i].x_offset,
                .YOffset = GlyphPos[i].y_offset,
                .XAdvance = GlyphPos[i].x_advance,
                .YAdvance = GlyphPos[i].y_advance
            };
        }

        Result.Glyphs = DstGlyphs;
        Result.Positions = DstPos;
    }

    return Result;
}

void Text_Shaper_Free_Result(text_shape_result* Result, allocator* Allocator) {
    if(Result && Result->GlyphCount) {
        Allocator_Free_Memory(Allocator, (void*)Result->Glyphs);
    }
}