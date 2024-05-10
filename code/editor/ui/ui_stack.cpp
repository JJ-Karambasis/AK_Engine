internal uptr UI_Get_Stack_Entry_Size(ui_stack_type Type) {
    local_persist const uptr Sizes[] = {
        sizeof(ui_stack_parent),
        sizeof(ui_stack_child_layout_axis),
        sizeof(ui_stack_fixed_width),
        sizeof(ui_stack_fixed_height),
        sizeof(ui_stack_pref_width),
        sizeof(ui_stack_pref_height),
        sizeof(ui_stack_background_color),
        sizeof(ui_stack_font),
        sizeof(ui_stack_text_color),
        sizeof(ui_stack_text_alignment)
    };
    static_assert(Array_Count(Sizes) == UI_STACK_TYPE_COUNT);
    Assert(Type < UI_STACK_TYPE_COUNT);
    return Sizes[Type];
}

internal ui_stack_list* UI_Get_Stack_List(ui* UI, ui_stack_type Type) {
    static_assert(Array_Count(UI->Stacks) == UI_STACK_TYPE_COUNT);
    Assert(Type < UI_STACK_TYPE_COUNT);
    return UI->Stacks + Type;
}

internal void UI_Stack_List_Push(ui_stack_list* List, ui_stack_entry* Entry) {
    DLL_Push_Back(List->First, List->Last, Entry);
}

internal void UI_Stack_List_Pop(ui_stack_list* List) {
    DLL_Remove_Back(List->First, List->Last);
}

internal ui_stack_entry* UI_Allocate_Stack_Entry(ui* UI, ui_stack_type Type) {
    ui_stack_entry* Entry = (ui_stack_entry*)Arena_Push(UI_Build_Arena(UI), UI_Get_Stack_Entry_Size(Type));
    return Entry;
}

internal ui_stack_entry* UI_Push_Stack(ui* UI, ui_stack_type Type, b32 AutoPop = false) {
    ui_stack_entry* Entry = UI_Allocate_Stack_Entry(UI, Type);
    ui_stack_list* StackList = UI_Get_Stack_List(UI, Type);
    StackList->AutoPop = AutoPop;
    UI_Stack_List_Push(StackList, Entry);
    return Entry;
}

internal void UI_Reset_Stacks(ui* UI) {
    for(u32 i = 0; i < UI_STACK_TYPE_COUNT; i++) {
        ui_stack_list* Stack = UI->Stacks + i;
        Stack->First = Stack->Last = nullptr;
    }

    //Push default stack values
    UI_Push_Pref_Width(UI, UI_Pixels(250.0f, 1.0f));
    UI_Push_Pref_Height(UI, UI_Pixels(30.0f, 1.0f));
    UI_Push_Background_Color(UI, vec4(1.0f, 0.0f, 1.0f, 1.0f));
    UI_Push_Text_Color(UI, vec4(1.0f, 0.0f, 1.0f, 1.0f));
    UI_Push_Text_Alignment(UI, UI_TEXT_ALIGNMENT_LEFT_BIT|UI_TEXT_ALIGNMENT_TOP_BIT);
}

#ifdef DEBUG_BUILD
internal void UI_Validate_Stacks(ui* UI) {
    local_persist const bool DebugSingleValidate[] = {
        true,
        false,
        false,
        false,
        true, 
        true,
        true,
        false,
        true,
        true
    };
    static_assert(Array_Count(DebugSingleValidate) == UI_STACK_TYPE_COUNT);

    for(u32 i = 0; i < UI_STACK_TYPE_COUNT; i++) {
        ui_stack_list* Stack = UI->Stacks + i;
        if(DebugSingleValidate[i]) {
            //Assert we have one entry
            Assert(Stack->First == Stack->Last);
        } else {
            //Assert we have no entries
            Assert(Stack->First == nullptr && Stack->Last == nullptr);
        }
    }
}
#else
#define UI_Validate_Stacks(...)
#endif

internal void UI_Autopop_Stacks(ui* UI) {
    for(u32 i = 0; i < UI_STACK_TYPE_COUNT; i++) {
        ui_stack_list* Stack = UI->Stacks + i;
        if(Stack->AutoPop) {
            UI_Stack_List_Pop(Stack);
            Stack->AutoPop = false;
        }
    }
}