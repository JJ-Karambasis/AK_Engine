internal uptr IM_Get_Stack_Entry_Size(u32 Type) {
    Assert(Type < im_stack_type::Count);
    return im_stack_size::Sizes[Type];
}

internal im_stack_list* IM_Get_Stack_List(im_renderer* Renderer, u32 Type) {
    Assert(Type < im_stack_type::Count);
    return Renderer->Stacks + Type;
}

internal void IM_Stack_List_Push(im_stack_list* List, im_stack_entry* Entry) {
    DLL_Push_Back(List->First, List->Last, Entry);
}

internal void IM_Stack_List_Pop(im_stack_list* List) {
    DLL_Remove_Back(List->First, List->Last);
}

internal im_stack_entry* IM_Allocate_Stack_Entry(im_renderer* Renderer, u32 Type) {
    im_stack_entry* Entry = (im_stack_entry*)Arena_Push(Renderer->RenderArena, IM_Get_Stack_Entry_Size(Type));
    return Entry;
}

internal im_stack_entry* IM_Push_Stack(im_renderer* Renderer, u32 Type, b32 AutoPop = false) {
    im_stack_entry* Entry = IM_Allocate_Stack_Entry(Renderer, Type);
    im_stack_list* StackList = IM_Get_Stack_List(Renderer, Type);
    StackList->AutoPop = AutoPop;
    IM_Stack_List_Push(StackList, Entry);
    return Entry;
}

internal void IM_Reset_Stacks(im_renderer* Renderer) {
    for(u32 i = 0; i < im_stack_type::Count; i++) {
        im_stack_list* Stack = Renderer->Stacks + i;
        //Ensure the stacks are empty
        Assert(Stack->First == nullptr && Stack->Last == nullptr); 
    }
}

internal void IM_Autopop_Stacks(im_renderer* Renderer) {
    for(u32 i = 0; i < im_stack_type::Count; i++) {
        im_stack_list* Stack = Renderer->Stacks + i;
        if(Stack->AutoPop) {
            IM_Stack_List_Pop(Stack);
            Stack->AutoPop = false;
        }
    }
}