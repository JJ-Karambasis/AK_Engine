#ifndef ATLAS_ALLOCATOR_H
#define ATLAS_ALLOCATOR_H

struct atlas_index {
    rect2i Rect;
};

struct atlas_alloc_id {
    u64          Generation;
    atlas_index* Index;
};

#define Atlas_Alloc_Is_Null(id) (!(id).Generation || !(id).Index)

enum atlas_allocator_free_list_type {
    ATLAS_ALLOCATOR_FREE_LIST_TYPE_SMALL,
    ATLAS_ALLOCATOR_FREE_LIST_TYPE_MEDIUM,
    ATLAS_ALLOCATOR_FREE_LIST_TYPE_LARGE,
    ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT
};

enum atlas_allocator_node_type {
    ATLAS_ALLOCATOR_NODE_TYPE_NONE,
    ATLAS_ALLOCATOR_NODE_TYPE_FREE,
    ATLAS_ALLOCATOR_NODE_TYPE_ALLOC,
    ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER
};

struct atlas_allocator_node : public atlas_index {
    atlas_allocator_node*     Parent;
    atlas_allocator_node*     Next;
    atlas_allocator_node*     Prev;
    u64                       Generation;
    b32                       IsVertical;
    atlas_allocator_node_type Type;

#ifdef DEBUG_BUILD
    atlas_allocator_node* NextInList;
#endif
};

struct atlas_allocator {
    arena*                       Arena;
    dim2i                        Size;
    vec2i                        Alignment;
    s32                          SmallThreshold;
    s32                          LargeThreshold;
    array<atlas_allocator_node*> FreeLists[ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT];
    array<atlas_allocator_node*> OrphanList;
    atlas_allocator_node*        RootNode;

#ifdef DEBUG_BUILD
    atlas_allocator_node* AllocatedList;
#endif
};

struct atlas_allocator_create_info {
    allocator* Allocator = Core_Get_Base_Allocator();
    dim2i      Dim;
    vec2i      Alignment = vec2i(4, 4);
    s32        SmallSizeThreshold = 32;
    s32        LargeSizeThreshold = 256;
};

atlas_allocator* Atlas_Allocator_Create(const atlas_allocator_create_info& CreateInfo);
void             Atlas_Allocator_Delete(atlas_allocator* Allocator);
atlas_alloc_id   Atlas_Allocator_Alloc(atlas_allocator* Allocator, dim2i Dim);
void             Atlas_Allocator_Free(atlas_allocator* Allocator, atlas_alloc_id AllocID);
atlas_index*     Atlas_Allocator_Get(atlas_allocator* Allocator, atlas_alloc_id AllocID);

#endif