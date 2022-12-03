void U32_List_Push_Node(u32_list* List, u32_node* Node)
{
    SLL_Push_Back(List->First, List->Last, Node);
    List->Count++;
}

void U32_List_Push(u32_list* List, allocator* Allocator, uint32_t Value)
{
    u32_node* Node = Allocate_Struct(Allocator, u32_node);
    Node->Value = Value;
    U32_List_Push_Node(List, Node);
}