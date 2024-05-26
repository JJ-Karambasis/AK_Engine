

#include "ui_common.cpp"
#include "ui_stack.cpp"

internal void UI_Box_Clear_Per_Frame(ui_box* Box) {
    Box->FirstChild = Box->LastChild = Box->NextSibling = Box->PrevSibling = Box->Parent = nullptr;
    Box->Flags = 0;
    Box->ChildCount = 0;
}

internal void UI_Box_Add_Child(ui_box* Parent, ui_box* Child) {
    DLL_Push_Back_NP(Parent->FirstChild, Parent->LastChild, Child, NextSibling, PrevSibling);
    Child->Parent = Parent;
    Parent->ChildCount++;
}

internal void UI_Size_Calculate_Standalone(ui* UI, ui_box* Root, ui_axis2 Axis) {
    switch(Root->PrefSize[Axis].Type) {
        case UI_SIZE_TYPE_PIXELS: {
            Root->FixedSize.Data[Axis] = Root->PrefSize[Axis].Value;
        } break;

        case UI_SIZE_TYPE_TEXT: {
            f32 Padding = Root->PrefSize[Axis].Value;
            f32 TextSize = Root->Text.Dim.Data[Axis];
            Root->FixedSize.Data[Axis] = (Padding*2)+TextSize;
        } break;
    }

    //Recurse
    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Size_Calculate_Standalone(UI, Child, Axis);
    }
}

internal void UI_Size_Calculate_Upward_Dependent(ui* UI, ui_box* Root, ui_axis2 Axis) {
    //Perform for all sizes that are parent(upward) dependent
    //todo: noop right now

    //Recurse
    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Size_Calculate_Upward_Dependent(UI, Child, Axis);
    }
}

internal void UI_Size_Calculate_Downward_Dependent(ui* UI, ui_box* Root, ui_axis2 Axis) {
    //Recurse first. May depend of children that have similar properties
    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Size_Calculate_Downward_Dependent(UI, Child, Axis);
    }

    //Perform for all sizes that are child(downward) dependent
    //todo: noop right now
}

internal void UI_Layout_Enforce_Constraints(ui* UI, ui_box* Root, ui_axis2 Axis) {
    //Fixup children sizes that are not along the main axis layout
    if(Root->ChildLayoutAxis != Axis) {
        f32 AllowedSize = Root->FixedSize.Data[Axis];
        for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
            f32 ChildSize = Child->FixedSize.Data[Axis];
            f32 Violation = ChildSize-AllowedSize;
            f32 MaxFixup = ChildSize;
            f32 Fixup = Clamp(0, Violation, MaxFixup);
            if(Fixup > 0) {
                Child->FixedSize.Data[Axis] -= Fixup;
            }
        }
    }

    //Fixup children sizes that are along the main axis layout
    if(Root->ChildLayoutAxis == Axis) {
        f32 TotalAllowedSize = Root->FixedSize.Data[Axis];
        
        f32 TotalSize = 0;
        f32 TotalWeightedSize = 0;
        for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
            TotalSize += Child->FixedSize.Data[Axis];
            TotalWeightedSize += Child->FixedSize.Data[Axis] * (1.0f-Child->PrefSize[Axis].Strictness);
        }

        //If we have a violation, we need to subtract some amount from all children
        f32 Violation = TotalSize-TotalAllowedSize;
        if(Violation > 0) {
            scratch Scratch = Scratch_Get();

            //Find out how much all the children take in total
            fixed_array<f32> ChildFixups(&Scratch, Root->ChildCount);

            uptr ChildIndex = 0;
            for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling, ChildIndex++) {
                f32 FixupSizeThisChild = Child->FixedSize.Data[Axis] * (1.0f-Child->PrefSize[Axis].Strictness);
                FixupSizeThisChild = Max(0, FixupSizeThisChild);
                ChildFixups[ChildIndex] = FixupSizeThisChild;
            }

            //Fixup child sizes
            ChildIndex = 0;
            for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling, ChildIndex++) {
                f32 FixupPercent = (Violation/TotalWeightedSize);
                FixupPercent = Clamp(0, FixupPercent, 1);
                Child->FixedSize.Data[Axis] -= ChildFixups[ChildIndex]*FixupPercent;
            }
        } 
    }

    //Now fixup parent(upwards) relative sizes, since the previous checks can rearrange things
    //todo: Right now we have none, we will need to at some point

    //Recurse
    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Layout_Enforce_Constraints(UI, Child, Axis);
    }
}

internal void UI_Layout_Positions(ui* UI, ui_box* Root, ui_axis2 Axis) {
    f32 LayoutPosition = 0.0f;

    //Layout children
    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        Child->FixedPosition.Data[Axis] = LayoutPosition;
        if(Root->ChildLayoutAxis == Axis) {
            LayoutPosition += Child->FixedSize.Data[Axis];
        } 

        Child->Rect.P1.Data[Axis] = Root->Rect.P1.Data[Axis] + Child->FixedPosition.Data[Axis];
        Child->Rect.P2.Data[Axis] = Child->Rect.P1.Data[Axis] + Child->FixedSize.Data[Axis];

        Child->Rect.P1.x = Floor_F32(Child->Rect.P1.x);
        Child->Rect.P1.y = Floor_F32(Child->Rect.P1.y);
        Child->Rect.P2.x = Floor_F32(Child->Rect.P2.x);
        Child->Rect.P2.y = Floor_F32(Child->Rect.P2.y);
    }

    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Layout_Positions(UI, Child, Axis);
    }
}

internal void UI_Layout_Root(ui* UI, ui_box* Root, ui_axis2 Axis) {
    UI_Size_Calculate_Standalone(UI, Root, Axis);
    UI_Size_Calculate_Upward_Dependent(UI, Root, Axis);
    UI_Size_Calculate_Downward_Dependent(UI, Root, Axis);
    UI_Layout_Enforce_Constraints(UI, Root, Axis);
    UI_Layout_Positions(UI, Root, Axis);
}

internal f32 UI_Get_Padding(ui_size Size) {
    f32 Result = 0.0f;
    if(Size.Type == UI_SIZE_TYPE_TEXT) {
        Result = Size.Value;
    }
    return Result;
}

internal bool UI_Font_Is_Different(ui_font* FontA, ui_font* FontB) {
    return (FontA->Font != FontB->Font || 
            FontA->Direction != FontB->Direction || 
            FontA->Script != FontB->Script || 
            FontA->Language != FontB->Language);
}

internal bool UI_Font_Is_Valid(ui_font* Font) {
    return (Font->Font.Is_Valid() && 
            Font->Direction != UBA_DIRECTION_INVALID && 
            Font->Script != UBA_SCRIPT_NONE);
}

internal bool UI_Text_Is_Valid(ui_box* Box, ui_text* Text) {
    return (!String_Is_Null_Or_Empty(Text->Text) &&
            UI_Font_Is_Valid(&Box->Font) &&
            Text->Count && Text->Glyphs);
}

internal void UI_Copy_Text(ui* UI, ui_box* Box, string Text) {
    ui_text* DisplayText = &Box->Text;
    Assert(DisplayText->Count > 0);

    DisplayText->Text = string(UI_Build_Arena(UI), Text);
    ui_text_glyph* Glyphs = Arena_Push_Array(UI_Build_Arena(UI), DisplayText->Count, ui_text_glyph);

    for(uptr i = 0; i < DisplayText->Count; i++) {
        Glyphs[i] = DisplayText->Glyphs[i];
    }

    DisplayText->Glyphs = Glyphs;
}

internal void UI_Shape(ui* UI, ui_box* Box, string Text) {
    scratch Scratch = Scratch_Get();
    ui_text* DisplayText = &Box->Text;

    DisplayText->Text = string(UI_Build_Arena(UI), Text);
    DisplayText->Font = Box->Font;
    Assert(UI_Font_Is_Valid(&DisplayText->Font));

    ui_font* Font = &DisplayText->Font;
    
    u32 Ascender = Font_Get_Metrics(Font->Font)->Ascender;
    text_shape_result ShapeResult = Font_Shape(Font->Font, {
        .Allocator = &Scratch,
        .Text = DisplayText->Text,
        .Properties = {
            .Direction = Font->Direction,
            .Script = Font->Script,
            .Language = Font->Language
        }
    });

    DisplayText->Count = ShapeResult.GlyphCount;
    DisplayText->Glyphs = Arena_Push_Array(UI_Build_Arena(UI), DisplayText->Count, ui_text_glyph);
    
    const text_glyph_info* Glyphs = ShapeResult.Glyphs;
    const text_shape_pos* Positions = ShapeResult.Positions;

    //Only LTR text is supported right now
    dim2 Dim = {};

    point2 Cursor = {};
    for(u32 i = 0; i < ShapeResult.GlyphCount; i++) {
        glyph_metrics Metrics = Font_Get_Glyph_Metrics(Font->Font, Glyphs[i].Codepoint);

        if(i != 0) {
            vec2i Kerning = Font_Get_Kerning(Font->Font, Glyphs[i-1].Codepoint, Glyphs[i].Codepoint);
            Cursor.x += (f32)Kerning.x;
        }

        ui_text_glyph* Glyph = DisplayText->Glyphs + i;
        //todo: Not sure if our offsets are properly calculated when the
        //text shaper offsets are not zero
        vec2 Offset = vec2(Positions[i].Offset + Metrics.Offset);
        Offset.y = (f32)Ascender - Offset.y;
        point2 PixelP = Cursor + Offset;

        Glyph->Codepoint = Glyphs[i].Codepoint;
        Glyph->ScreenRect = rect2(PixelP, PixelP+dim2(Metrics.Dim));

        //If this is the last entry in the shape, the size of the shapes
        //combined is position of the last glyph plus the width of the last shape.
        //This just ends up being the final glyphs ScreenRect.x since we always
        //start from zero
        if(i == (ShapeResult.GlyphCount-1)) {
            Dim.width = Glyph->ScreenRect.P2.x;
        }
        Dim.height = Max(Dim.height, Glyph->ScreenRect.P2.y);

        Cursor.x += (f32)Metrics.Advance.x;
    } 
    DisplayText->Dim = Dim;
}

void UI_Render_Root(ui* UI, ui_box* Root, im_renderer* Renderer) {
    if(Root->CustomRenderFunc) {
        Root->CustomRenderFunc(UI, Root, Renderer, Root->RenderFuncUserData);
    }

    IM_Draw_Quad(Renderer, Root->Rect, {}, Root->BackgroundColor);

    if(Root->Flags & UI_BOX_FLAG_DRAW_TEXT) {
        ui_text* Text = &Root->Text;

        if(UI_Text_Is_Valid(Root, Text)) {
            vec2 Offset = {};

            for(u32 i = 0; i < UI_AXIS2_COUNT; i++) {
                u32 Index = i*3;

                f32 Padding = UI_Get_Padding(Root->PrefSize[i]);
                f32 Max = Max(Root->Rect.P2.Data[i]-Padding, 0.0f);
                f32 Min = Min(Root->Rect.P1.Data[i]+Padding, Max);

                if(Root->TextAlignment & (1 << Index)) {
                    Assert(!(Root->TextAlignment & (1 << (Index+1))));
                    Assert(!(Root->TextAlignment & (1 << (Index+2))));
                    
                    Offset.Data[i] = Min;
                } else if(Root->TextAlignment & (1 << (Index+1))) {
                    Assert(!(Root->TextAlignment & (1 << (Index+2))));

                    Offset.Data[i] = Max - Text->Dim.Data[i];
                } else if(Root->TextAlignment & (1 << (Index+2))) {
                    f32 TextHalf = Text->Dim.Data[i]*0.5f;
                    f32 Half = (Max-Min)*0.5f;
                    Offset.Data[i] = Min + ((Half-Text->Dim.Data[i])+TextHalf);
                } else {
                    //Please specify a text alignment
                    Assert(false);
                }
            }

            for(u32 i = 0; i < Text->Count; i++) {
                ui_text_glyph* Glyph = Text->Glyphs + i;
                const glyph_entry* CacheGlyph = Glyph_Cache_Get(UI->GlyphCache, Text->Font.Font, Glyph->Codepoint);
                if(CacheGlyph) {
                    IM_Draw_Quad(Renderer, Glyph->ScreenRect+Offset, CacheGlyph->UVRect, Root->TextColor);
                }
            }
        }
    }

    for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling) {
        UI_Render_Root(UI, Child, Renderer);
    }
}

IM_CALLBACK_DEFINE(UI_Render) {
    ui* UI = (ui*)UserData;

    ui_global_data GlobalData {
        .Projection = Transpose(Ortho_Projection2D(Resolution.width, Resolution.height))
    };


    GDI_Context_Buffer_Write(IM_Context(Renderer), UI->GlobalBuffer, &GlobalData);

    const renderer_texture* AtlasTexture = Glyph_Cache_Get_Atlas(UI->GlyphCache);

    IM_Push_Pipeline(Renderer, UI->Pipeline);
    IM_Push_Bind_Group(Renderer, 0, UI->GlobalBindGroup);
    IM_Push_Bind_Group(Renderer, 1, AtlasTexture->BindGroup);

    UI_Render_Root(UI, UI->Root, Renderer);

    IM_Pop_Bind_Group(Renderer, 1);
    IM_Pop_Bind_Group(Renderer, 0);
    IM_Pop_Pipeline(Renderer);
}

ui* UI_Create(const ui_create_info& CreateInfo) {
    arena* Arena = Arena_Create(CreateInfo.Allocator, KB(64));
    ui* UI = Arena_Push_Struct(Arena, ui);
    UI->Arena = Arena;
    UI->GlyphCache = CreateInfo.GlyphCache;
    UI->Pipeline = CreateInfo.Pipeline;
    
    UI->Renderer = IM_Create({
        .Allocator = Arena,
        .Renderer = CreateInfo.Renderer,
        .Callback = UI_Render,
        .UserData = UI
    });

    gdi_context* Context = IM_Context(UI->Renderer);
    UI->GlobalBuffer = GDI_Context_Create_Buffer(Context, {
        .ByteSize = sizeof(ui_global_data),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT 
    });

    UI->GlobalBindGroup = GDI_Context_Create_Bind_Group(Context, {
        .Layout = CreateInfo.GlobalLayout,
        .WriteInfo = {
            .Bindings = { {
                    .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                    .BufferBinding = {UI->GlobalBuffer}
                }
            }
        }
    });

    UI->BuildArenas[0] = Arena_Create(UI->Arena);
    UI->BuildArenas[1] = Arena_Create(UI->Arena);

    Hashmap_Init(&UI->BoxHashTable, UI->Arena);
    return UI;
}

void UI_Delete(ui* UI) {
    if(UI && UI->Arena) {
        arena* Arena = UI->Arena;
        Arena_Delete(Arena);
    }
}

void UI_Begin_Build(ui* UI, window_handle WindowHandle, editor_input_manager* InputManager, const point2i& MousePos) {
    UI_Reset_Stacks(UI);
    UI->InputManager = InputManager;
    UI->MousePosition = MousePos;

    window* Window = Window_Get(WindowHandle);

    //First setup top level root
    {
        UI_Set_Next_Fixed_Width(UI, (f32)Window->Size.width);
        UI_Set_Next_Fixed_Height(UI, (f32)Window->Size.height);
        UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_X);
        UI->Root = UI_Build_Box_From_StringF(UI, 0, "###%llu_root__", (u64)(uptr)Window);
    }

    //Setup top level default stacks
    {
        UI_Push_Parent(UI, UI->Root);
    }
}

void UI_End_Build(ui* UI) {
    //Check and remove old entries from the cache
    UI_Validate_Stacks(UI);

    {
        hashmap<ui_key, ui_box*>* BoxHashTable = &UI->BoxHashTable;
        u32 i = 0;
        while(i < BoxHashTable->Count) {
            ui_box* Box = BoxHashTable->Values[i];
            if(Box->LastUsedBuildIndex < UI->BuildIndex) {
                //Key and hash are the same
                Hashmap_Remove_By_Hash(BoxHashTable, Box->Key, Box->Key);
                SLL_Push_Front_N(UI->FirstFreeBox, Box, NextSibling);
            } else {
                i++;
            }
        }
    }

    //Layout box tree
    for(u32 i = 0; i < UI_AXIS2_COUNT; i++) {
        UI_Layout_Root(UI, UI->Root, (ui_axis2)i);
    }

    UI->BuildIndex++;
    Arena_Clear(UI_Build_Arena(UI));
}

ui_box* UI_Box_From_Key(ui* UI, ui_key Key) {
    ui_box** pBox = Hashmap_Find_By_Hash(&UI->BoxHashTable, Key, Key);
    if(!pBox) return nullptr;
    return *pBox;
}

ui_box* UI_Current_Box(ui* UI) {
    return UI->CurrentBox;
}

ui_box* UI_Build_Box_From_Key(ui* UI, ui_box_flags Flags, ui_key Key) {
    //Check to see if our box is allocated
    ui_box* Box = UI_Box_From_Key(UI, Key);
    b32 IsAllocated = Box != nullptr;

    if(!IsAllocated) {
        //If its not allocated, allocate a new box
        Box = UI->FirstFreeBox;
        if(Box) SLL_Pop_Front_N(UI->FirstFreeBox, NextSibling);
        else Box = Arena_Push_Struct_No_Clear(UI->Arena, ui_box);
        Zero_Struct(Box);
    }

    //Zero out per frame box state
    UI_Box_Clear_Per_Frame(Box);
        
    //Add the box into the hash table if we just created it
    if(!IsAllocated) {
        Hashmap_Add_By_Hash(&UI->BoxHashTable, Key, Key, Box);
    }

    //Add into the ui tree
    ui_stack_parent* Parent = UI_Current_Parent(UI);
    if(Parent) {
        UI_Box_Add_Child(Parent->Value, Box);
    }

    //Per frame updates
    {
        Box->Key = Key;
        Box->Flags = Flags;
        Box->LastUsedBuildIndex = UI->BuildIndex;
        if(ui_stack_fixed_width* FixedWidth = UI_Current_Fixed_Width(UI)) {
            Box->Flags |= UI_BOX_FLAG_FIXED_WIDTH_BIT;
            Box->FixedSize.width = FixedWidth->Value;
        } else {
            ui_stack_pref_width* PrefWidth = UI_Current_Pref_Width(UI);
            Box->PrefSize[UI_AXIS2_X] = PrefWidth->Value;
        }

        if(ui_stack_fixed_height* FixedHeight = UI_Current_Fixed_Height(UI)) {
            Box->Flags |= UI_BOX_FLAG_FIXED_HEIGHT_BIT;
            Box->FixedSize.height = FixedHeight->Value;
        } else {
            ui_stack_pref_height* PrefHeight = UI_Current_Pref_Height(UI);
            Box->PrefSize[UI_AXIS2_Y] = PrefHeight->Value;
        }

        if(Box->Flags & UI_BOX_FLAG_DRAW_TEXT) {
            Box->TextColor = UI_Current_Text_Color(UI)->Value;
            Box->TextAlignment = UI_Current_Text_Alignment(UI)->Value;

            ui_stack_font* Font = UI_Current_Font(UI);
            if(Font) {
                Box->Font = Font->Value;
            }
        }

        Box->BackgroundColor = UI_Current_Background_Color(UI)->Value;
    }

    UI_Autopop_Stacks(UI);

    UI->CachedSignal = {};
    UI->CurrentBox = Box;

    return Box;
}

ui_box* UI_Build_Box_From_String(ui* UI, ui_box_flags Flags, string String) {
    ui_key ParentKey = 0;
    if(UI_Current_Parent(UI)) {
        ParentKey = UI_Current_Parent(UI)->Value->Key;
    }

    ui_key Key = UI_Key_From_String(ParentKey, String);
    ui_box* Box = UI_Build_Box_From_Key(UI, Flags, Key);

    if(Flags & UI_BOX_FLAG_DRAW_TEXT) {
        string DisplayPart = UI_Display_Part_From_String(String);
        UI_Box_Attach_Display_Text(UI, Box, DisplayPart);
    }

    return Box;
}

ui_box* UI_Build_Box_From_StringF(ui* UI, ui_box_flags Flags, const char* Format, ...) {
    scratch Scratch = Scratch_Get();

    va_list Args;
    va_start(Args, Format);
    string String(&Scratch, Format, Args);
    va_end(Args);

    ui_box* Box = UI_Build_Box_From_String(UI, Flags, String);
    return Box;   
}

void UI_Box_Attach_Custom_Render(ui_box* Box, ui_custom_render_func* RenderFunc, void* UserData) {
    Box->CustomRenderFunc = RenderFunc;
    Box->RenderFuncUserData = UserData;
}

void UI_Box_Attach_Display_Text(ui* UI, ui_box* Box, string Text) {
    //First make sure the font that is attached to the box is valid
    ui_font* Font = &Box->Font;
    if(!UI_Font_Is_Valid(Font)) {
        Assert(false);
        return;
    }

    //Then, we are going to be setting properties on the 
    //display text to see if we need to reshape or not
    ui_text* DisplayText = &Box->Text;
    bool Reshape = false;

    //The normal text is first    
    if(DisplayText->Text != Text) {
        Reshape = true;
    }

    //Then if the font is different we also have to reshape
    if(UI_Font_Is_Different(Font, &DisplayText->Font)) {
        Reshape = true;
    }

    //Reshape or copy the text
    if(Reshape) {
        UI_Shape(UI, Box, Text);
    } else {
        //If we don't need to reshape, copy the text next frame
        UI_Copy_Text(UI, Box, Text);
    }
}

const ui_text* UI_Box_Get_Display_Text(ui_box* Box) {
    Assert(!String_Is_Null_Or_Empty(Box->Text.Text));
    return &Box->Text;
}

dim2 UI_Box_Get_Dim(ui_box* Box) {
    return Box->FixedSize;
}

ui_signal UI_Signal_From_Box(ui* UI, ui_box* Box) {
    ui_signal Result = {};
    editor_input_manager* InputManager = UI->InputManager;

    bool ShouldHoverCheck = Box->Flags & UI_BOX_FLAG_MOUSE_CLICKABLE; 

    if(ShouldHoverCheck) {
        if(Rect2_Contains_Point(Box->Rect, point2(UI->MousePosition))) {
            Result.Flags |= UI_SIGNAL_FLAG_HOVERING_BIT;
        }
    }

    if(Box->Flags & UI_BOX_FLAG_MOUSE_CLICKABLE) {
        //Make sure we are hovering before recording click commands 
        if(Result.Flags & UI_SIGNAL_FLAG_HOVERING_BIT) {
            if(InputManager->Is_Mouse_Pressed(OS_MOUSE_KEY_LEFT)) {
                Result.Flags |= UI_SIGNAL_FLAG_LEFT_PRESSED_BIT;
            } 
            
            if(InputManager->Is_Mouse_Down(OS_MOUSE_KEY_LEFT)) {
                Result.Flags |= UI_SIGNAL_FLAG_LEFT_DOWN_BIT;
            }

            if(InputManager->Is_Mouse_Released(OS_MOUSE_KEY_LEFT)) {
                Result.Flags |= UI_SIGNAL_FLAG_LEFT_RELEASED_BIT;
            }
        }
    }

    Result.Box = Box;
    return Result;
}

ui_signal UI_Current_Signal(ui* UI) {
    if(!UI->CachedSignal.Box) {
        UI->CachedSignal = UI_Signal_From_Box(UI, UI_Current_Box(UI));
    }
    return UI->CachedSignal;
}

#define UI_Current_Stack_Entry(constant, type) (type*)(UI->Stacks[constant].Last)

#define UI_Pop_Type(type) UI_Stack_List_Pop(&UI->Stacks[type])
#define UI_Pop_Type_Safe(type) UI_Stack_List_Pop(&UI->Stacks[type]); Assert(UI->Stacks[type].Last)

#define UI_Push_Type(constant, type, data_value) \
    type* Entry = (type*)UI_Push_Stack(UI, constant); \
    Entry->Value = data_value

#define UI_Autopush_Type(constant, type, data_value) \
    type* Entry = (type*)UI_Push_Stack(UI, constant, true); \
    Entry->Value = data_value

//UI push stack API
void UI_Push_Parent(ui* UI, ui_box* Box) {
    UI_Push_Type(UI_STACK_TYPE_PARENT, ui_stack_parent, Box);
}

void UI_Push_Child_Layout_Axis(ui* UI, ui_axis2 Axis) {
    UI_Push_Type(UI_STACK_TYPE_CHILD_LAYOUT_AXIS, ui_stack_child_layout_axis, Axis);
}

void UI_Push_Fixed_Width(ui* UI, f32 Width) {
    UI_Push_Type(UI_STACK_TYPE_FIXED_WIDTH, ui_stack_fixed_width, Width);
}

void UI_Push_Fixed_Height(ui* UI, f32 Height) {
    UI_Push_Type(UI_STACK_TYPE_FIXED_HEIGHT, ui_stack_fixed_height, Height);
}

void UI_Push_Pref_Width(ui* UI, ui_size Size) {
    UI_Push_Type(UI_STACK_TYPE_PREF_WIDTH, ui_stack_pref_width, Size);
}

void UI_Push_Pref_Height(ui* UI, ui_size Size) {
    UI_Push_Type(UI_STACK_TYPE_PREF_HEIGHT, ui_stack_pref_height, Size);
}

void UI_Push_Background_Color(ui* UI, color4 Color) {
    UI_Push_Type(UI_STACK_TYPE_BACKGROUND_COLOR, ui_stack_background_color, Color);
}

void UI_Push_Font(ui* UI, ui_font Font) {
    UI_Push_Type(UI_STACK_TYPE_FONT, ui_stack_font, Font);
}

void UI_Push_Text_Color(ui* UI, color4 Color) {
    UI_Push_Type(UI_STACK_TYPE_TEXT_COLOR, ui_stack_text_color, Color);
}

void UI_Push_Text_Alignment(ui* UI, ui_text_alignment_flags TextAlignment) {
    UI_Push_Type(UI_STACK_TYPE_TEXT_ALIGNMENT, ui_stack_text_alignment, TextAlignment);
}

//UI pop stack API
void UI_Pop_Parent(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_PARENT);
}

void UI_Pop_Child_Layout_Axis(ui* UI) {
    UI_Pop_Type(UI_STACK_TYPE_CHILD_LAYOUT_AXIS);
}

void UI_Pop_Fixed_Width(ui* UI) {
    UI_Pop_Type(UI_STACK_TYPE_FIXED_WIDTH);
}

void UI_Pop_Fixed_Height(ui* UI) {
    UI_Pop_Type(UI_STACK_TYPE_FIXED_HEIGHT);
}

void UI_Pop_Pref_Width(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_PREF_WIDTH);
}

void UI_Pop_Pref_Height(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_PREF_HEIGHT);
}

void UI_Pop_Background_Color(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_BACKGROUND_COLOR);
}

void UI_Pop_Font(ui* UI) {
    UI_Pop_Type(UI_STACK_TYPE_FONT);
}

void UI_Pop_Text_Color(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_TEXT_COLOR);
}

void UI_Pop_Text_Alignment(ui* UI) {
    UI_Pop_Type_Safe(UI_STACK_TYPE_TEXT_ALIGNMENT);
}

//UI autopop api
void UI_Set_Next_Parent(ui* UI, ui_box* Box) {
    UI_Autopush_Type(UI_STACK_TYPE_PARENT, ui_stack_parent, Box);
}

void UI_Set_Next_Child_Layout_Axis(ui* UI, ui_axis2 Axis) {
    UI_Autopush_Type(UI_STACK_TYPE_CHILD_LAYOUT_AXIS, ui_stack_child_layout_axis, Axis);
}

void UI_Set_Next_Fixed_Width(ui* UI, f32 Width) {
    UI_Autopush_Type(UI_STACK_TYPE_FIXED_WIDTH, ui_stack_fixed_width, Width);
}

void UI_Set_Next_Fixed_Height(ui* UI, f32 Height) {
    UI_Autopush_Type(UI_STACK_TYPE_FIXED_HEIGHT, ui_stack_fixed_height, Height);
}

void UI_Set_Next_Pref_Width(ui* UI, ui_size Size) {
    UI_Autopush_Type(UI_STACK_TYPE_PREF_WIDTH, ui_stack_pref_width, Size);
}

void UI_Set_Next_Pref_Height(ui* UI, ui_size Size) {
    UI_Autopush_Type(UI_STACK_TYPE_PREF_HEIGHT, ui_stack_pref_height, Size);
}

void UI_Set_Next_Background_Color(ui* UI, color4 Color) {
    UI_Autopush_Type(UI_STACK_TYPE_BACKGROUND_COLOR, ui_stack_background_color, Color);
}

void UI_Set_Next_Font(ui* UI, ui_font Font) {
    UI_Autopush_Type(UI_STACK_TYPE_FONT, ui_stack_font, Font);
}

void UI_Set_Next_Text_Color(ui* UI, color4 Color) {
    UI_Autopush_Type(UI_STACK_TYPE_TEXT_COLOR, ui_stack_text_color, Color);
}

void UI_Set_Next_Text_Alignment(ui* UI, ui_text_alignment_flags AlignmentFlags) {
    UI_Autopush_Type(UI_STACK_TYPE_TEXT_ALIGNMENT, ui_stack_text_alignment, AlignmentFlags);
}

//UI get most recent stack api
ui_stack_parent* UI_Current_Parent(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_PARENT, ui_stack_parent);
}

ui_stack_child_layout_axis* UI_Current_Child_Layout(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_CHILD_LAYOUT_AXIS, ui_stack_child_layout_axis);
}

ui_stack_fixed_width* UI_Current_Fixed_Width(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_FIXED_WIDTH, ui_stack_fixed_width);
}

ui_stack_fixed_height* UI_Current_Fixed_Height(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_FIXED_HEIGHT, ui_stack_fixed_height);
}

ui_stack_pref_width* UI_Current_Pref_Width(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_PREF_WIDTH, ui_stack_pref_width);
}

ui_stack_pref_height* UI_Current_Pref_Height(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_PREF_HEIGHT, ui_stack_pref_height);
}

ui_stack_background_color* UI_Current_Background_Color(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_BACKGROUND_COLOR, ui_stack_background_color);
}

ui_stack_font* UI_Current_Font(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_FONT, ui_stack_font);
}

ui_stack_text_color* UI_Current_Text_Color(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_TEXT_COLOR, ui_stack_text_color);
}

ui_stack_text_alignment* UI_Current_Text_Alignment(ui* UI) {
    return UI_Current_Stack_Entry(UI_STACK_TYPE_TEXT_ALIGNMENT, ui_stack_text_alignment);
}
