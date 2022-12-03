#ifndef LISTS_H
#define LISTS_H

typedef struct u32_node
{
    uint32_t Value;
    struct u32_node* Next;
} u32_node;

typedef struct u32_list
{
    u32_node* First;
    u32_node* Last;
    uint64_t  Count;
} u32_list;

void U32_List_Push_Node(u32_list* List, u32_node* Node);
void U32_List_Push(u32_list* List, allocator* Allocator, uint32_t Value);

#endif
