

#include "ui_common.cpp"
#include "ui_stack.cpp"

internal void UI_Box_Clear_Per_Frame(ui_box* Box) {
    Box->FirstChild = Box->LastChild = Box->NextSibling = Box->PrevSibling = Box->Parent = nullptr;
    Box->Flags = 0;
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
            f32 ChildFixupSum = 0.0f;
            fixed_array<f32> ChildFixups(&Scratch, Root->ChildCount);

            uptr ChildIndex = 0;
            for(ui_box* Child = Root->FirstChild; Child; Child = Child->NextSibling, ChildIndex++) {
                f32 FixupSizeThisChild = Child->FixedSize.Data[Axis] * (1.0f-Child->PrefSize[Axis].Strictness);
                FixupSizeThisChild = Max(0, FixupSizeThisChild);
                ChildFixups[ChildIndex] = FixupSizeThisChild;
                ChildFixupSum += FixupSizeThisChild;
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

        Child->Rect.Min.Data[Axis] = Root->Rect.Min.Data[Axis] + Child->FixedPosition.Data[Axis];
        Child->Rect.Max.Data[Axis] = Child->Rect.Min.Data[Axis] + Child->FixedSize.Data[Axis];

        Child->Rect.Min.x = Floor_F32(Child->Rect.Min.x);
        Child->Rect.Min.y = Floor_F32(Child->Rect.Min.y);
        Child->Rect.Max.x = Floor_F32(Child->Rect.Max   .x);
        Child->Rect.Max.y = Floor_F32(Child->Rect.Max.y);
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

ui* UI_Create(const ui_create_info& CreateInfo) {
    arena* Arena = Arena_Create(CreateInfo.Allocator, KB(64));
    ui* UI = Arena_Push_Struct(Arena, ui);
    UI->Arena = Arena;

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

void UI_Begin_Build(ui* UI, window_handle WindowHandle) {
    UI_Reset_Stacks(UI);

    window* Window = Window_Get(WindowHandle);

    //First setup top level root
    {
        UI_Set_Next_Fixed_Width(UI, (f32)Window->Size.w);
        UI_Set_Next_Fixed_Height(UI, (f32)Window->Size.h);
        UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_X);
        UI->Root = UI_Build_Box_From_StringF(UI, 0, "###%I64x", (u64)(uptr)Window);
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

    UI->BuildIndex = UI->BuildIndex++;
    Arena_Clear(UI_Build_Arena(UI));
}

ui_box* UI_Box_From_Key(ui* UI, ui_key Key) {
    ui_box** pBox = Hashmap_Find_By_Hash(&UI->BoxHashTable, Key, Key);
    if(!pBox) return nullptr;
    return *pBox;
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
            Box->FixedSize.w = FixedWidth->Value;
        } else {
            ui_stack_pref_width* PrefWidth = UI_Current_Pref_Width(UI);
            Box->PrefSize[UI_AXIS2_X] = PrefWidth->Value;
        }

        if(ui_stack_fixed_height* FixedHeight = UI_Current_Fixed_Height(UI)) {
            Box->Flags |= UI_BOX_FLAG_FIXED_HEIGHT_BIT;
            Box->FixedSize.h = FixedHeight->Value;
        } else {
            ui_stack_pref_height* PrefHeight = UI_Current_Pref_Height(UI);
            Box->PrefSize[UI_AXIS2_Y] = PrefHeight->Value;
        }

        Box->BackgroundColor = UI_Current_Background_Color(UI)->Value;
    }

    UI_Autopop_Stacks(UI);

    return Box;
}

ui_box* UI_Build_Box_From_String(ui* UI, ui_box_flags Flags, string String) {
    ui_key ParentKey = 0;
    if(UI_Current_Parent(UI)) {
        ParentKey = UI_Current_Parent(UI)->Value->Key;
    }

    ui_key Key = UI_Key_From_String(ParentKey, String);
    ui_box* Box = UI_Build_Box_From_Key(UI, Flags, Key);
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

void UI_Push_Background_Color(ui* UI, vec4 Color) {
    UI_Push_Type(UI_STACK_TYPE_BACKGROUND_COLOR, ui_stack_background_color, Color);
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

void UI_Set_Next_Background_Color(ui* UI, vec4 Color) {
    UI_Autopush_Type(UI_STACK_TYPE_BACKGROUND_COLOR, ui_stack_background_color, Color);
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