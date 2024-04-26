internal atlas_allocator_free_list_type Atlas__Get_Free_List_Type(u32 SmallThreshold, u32 LargeThreshold, uvec2 Size) {
    if(Size.w >= LargeThreshold || Size.h >= LargeThreshold) {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_LARGE;
    } else if(Size.w >= SmallThreshold || Size.h >= SmallThreshold) {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_MEDIUM;
    } else {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_SMALL;
    }
}

internal atlas_allocator_node* Atlas__Create_Node(atlas_allocator* Allocator) {
    atlas_allocator_node* Result = nullptr;
    if(Allocator->OrphanList.Count) {
        Result = Array_Pop(&Allocator->OrphanList);
        Assert(Result->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE);
    } else {
        Result = Arena_Push_Struct(Allocator->Arena, atlas_allocator_node);
        Result->Type = ATLAS_ALLOCATOR_NODE_TYPE_NONE;
#ifdef DEBUG_BUILD
        SLL_Push_Front_N(Allocator->AllocatedList, Result, NextInList);
#endif    
    }
    Result->Generation++;
    return Result;
}

internal void Atlas__Delete_Node(atlas_allocator* Allocator, atlas_allocator_node* Node) {
    Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_NONE;
    Node->Next = nullptr;
    Array_Push(&Allocator->OrphanList, Node);
}

internal void Atlas__Add_Free_Rect(atlas_allocator* Allocator, atlas_allocator_node* Node, uvec2 Size) {
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    atlas_allocator_free_list_type Type = Atlas__Get_Free_List_Type(Allocator->SmallThreshold, Allocator->LargeThreshold, Size);
    Array_Push(&Allocator->FreeLists[Type], Node);
}

internal atlas_allocator_node* Atlas__Find_Free_Rect(atlas_allocator* Allocator, uvec2 RequestSize) {
    atlas_allocator_free_list_type ListType = Atlas__Get_Free_List_Type(Allocator->SmallThreshold, Allocator->LargeThreshold, RequestSize);
    bool UseWorstFit = ListType == ATLAS_ALLOCATOR_FREE_LIST_TYPE_LARGE;

    for(u32 ListIndex = ListType; ListIndex < ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT; ListIndex++) {
        s32 BestScore = UseWorstFit ? 0 : INT32_MAX;
        uptr BestIndex = (uptr)-1;
        atlas_allocator_node* BestNode = nullptr;

        array<atlas_allocator_node*>& FreeList = Allocator->FreeLists[ListIndex];
        uptr FreeListIndex = 0;
        while(FreeListIndex < FreeList.Count) {
            atlas_allocator_node* FreeNode = FreeList[FreeListIndex];
            if(FreeNode->Type != ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Array_Swap_Remove(&FreeList, FreeListIndex);
                continue;
            }

            Assert(FreeNode->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);

            uvec2 Size = Rect2u_Get_Dim(FreeNode->Rect);

            //Sizes are always unsigned, but subtracting the sizes can yield negatives
            s32 dx = (s32)Size.w - (s32)RequestSize.w;
            s32 dy = (s32)Size.h - (s32)RequestSize.h;

            if(dx >= 0 && dy >= 0) {
                if(dx == 0 || dy == 0) {
                    //Perfect fit!
                    BestNode = FreeNode;
                    BestIndex = FreeListIndex;
                    break;
                }

                s32 Score = Min(dx, dy);
                if((UseWorstFit && (Score > BestScore)) ||
                   (!UseWorstFit && (Score < BestScore))) {
                    BestScore = Score;
                    BestNode = FreeNode;
                    BestIndex = FreeListIndex;
                }
            }

            FreeListIndex++;
        }

        if(BestNode) {
            Array_Swap_Remove(&FreeList, BestIndex);
            return BestNode;
        }
    }

    return nullptr;

}

internal void Atlas__Merge_Siblings(atlas_allocator* Allocator, atlas_allocator_node* Node, atlas_allocator_node* Next, b32 IsVertical) {
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    Assert(Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    rect2u R1 = Node->Rect;
    rect2u R2 = Next->Rect;

    //Merge the nodes based on alignment
    uvec2 MergeSize = Rect2u_Get_Dim(R2);
    if(IsVertical) {
        Assert(R1.Min.x == R2.Min.x);
        Assert(R1.Max.x == R2.Max.x);
        Node->Rect.Max.y += MergeSize.h;
    } else {
        Assert(R1.Min.y == R2.Min.y);
        Assert(R1.Max.y == R2.Max.y);
        Node->Rect.Max.x += MergeSize.w;
    }

    //Remove the merged node from the sibling list
    atlas_allocator_node* NextNext = Next->Next;
    Node->Next = NextNext;
    if(NextNext) {
        NextNext->Prev = Node;
    }

    //Change its state so it can be cleaned up later
    Atlas__Delete_Node(Allocator, Next);
}

internal void Atlas__Guillotine_Rect(rect2u* ChosenRect, uvec2 RequestSize, b32 IsVertical, rect2u* OutSplitRect, rect2u* OutLeftoverRect, b32* OutIsVertical) {
    rect2u LeftoverRectToRight(
        ChosenRect->Min + uvec2(RequestSize.w, 0),
        uvec2(ChosenRect->Max.x, ChosenRect->Min.y+RequestSize.h)
    );

    rect2u LeftoverRectToBottom(
        ChosenRect->Min + uvec2(0, RequestSize.h),
        uvec2(ChosenRect->Min.x + RequestSize.w, ChosenRect->Max.y)
    );

    if(RequestSize == Rect2u_Get_Dim(*ChosenRect)) {
        *OutIsVertical = IsVertical;
        *OutSplitRect = Rect2u_Empty();
        *OutLeftoverRect = Rect2u_Empty();
    } else if(Rect2u_Area(LeftoverRectToRight) > Rect2u_Area(LeftoverRectToBottom)) {
        *OutLeftoverRect = LeftoverRectToBottom;
        *OutSplitRect = Rect2u(
            LeftoverRectToRight.Min,
            uvec2(LeftoverRectToRight.Max.x, ChosenRect->Max.y)
        );
        *OutIsVertical = false;
    } else {
        *OutLeftoverRect = LeftoverRectToRight;
        *OutSplitRect = Rect2u(
            LeftoverRectToBottom.Min,
            uvec2(ChosenRect->Max.x, LeftoverRectToBottom.Max.y)
        );
        *OutIsVertical = true;
    }
}

#ifdef DEBUG_BUILD

void Atlas__Check_Siblings(atlas_allocator_node* Node, atlas_allocator_node* Next, b32 IsVertical) {
    if(!Next) return;
    Assert(Node == Next->Prev);

    if(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE || Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER) {
        return;
    }

    if(Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE || Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER) {
        return;
    }

    rect2u R1 = Node->Rect;
    rect2u R2 = Next->Rect;
    if(IsVertical) {
        Assert(R1.Min.x == R2.Min.x);
        Assert(R1.Max.x == R2.Max.x);
    } else {
        Assert(R1.Min.y == R2.Min.y);
        Assert(R1.Max.y == R2.Max.y);
    }
}

void Atlas__Check_Tree(atlas_allocator* Allocator) {
    atlas_allocator_node* Node = Allocator->AllocatedList;
    while(Node) {
        if(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE) {
            Node = Node->NextInList;
            continue;
        }

        atlas_allocator_node* Iter = Node->Next;
        while(Iter) {
            Assert(Iter->IsVertical == Node->IsVertical);
            Assert(Iter->Parent == Node->Parent);
            Assert(Iter->Type != ATLAS_ALLOCATOR_NODE_TYPE_NONE);

            atlas_allocator_node* Next = Iter->Next;
            Atlas__Check_Siblings(Iter, Next, Node->IsVertical);

            Iter = Next;
        }

        if(Node->Parent) {
            Assert(Node->Parent->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER);
            Assert(Node->Parent->IsVertical == !Node->IsVertical);
        }

        Node = Node->NextInList;
    }
}
#endif

atlas_allocator* Atlas_Allocator_Create(const atlas_allocator_create_info& CreateInfo) {
    Assert(CreateInfo.Allocator);
    Assert(CreateInfo.Dim.w > 0);
    Assert(CreateInfo.Dim.h > 0);
    Assert(CreateInfo.Alignment.w > 0);
    Assert(CreateInfo.Alignment.h > 0);
    Assert(CreateInfo.LargeSizeThreshold >= CreateInfo.SmallSizeThreshold);
    
    arena* Arena = Arena_Create(CreateInfo.Allocator);

    atlas_allocator* Result = Arena_Push_Struct(Arena, atlas_allocator);
    Result->Arena = Arena;
    Result->Size = CreateInfo.Dim;
    Result->Alignment = CreateInfo.Alignment;
    Result->SmallThreshold = CreateInfo.SmallSizeThreshold;
    Result->LargeThreshold = CreateInfo.LargeSizeThreshold;
    for(u32 i = 0; i < ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT; i++) {
        Array_Init(&Result->FreeLists[i], Arena);
    }
    Array_Init(&Result->OrphanList, Arena);

    atlas_allocator_node* Node = Atlas__Create_Node(Result);
    Node->Rect                 = Rect2u_From_Dim(Result->Size);
    Node->IsVertical           = true;
    Node->Type                 = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
    Atlas__Add_Free_Rect(Result, Node, Result->Size);
    Result->RootNode = Node;

    return Result;
}

void             Atlas_Allocator_Delete(atlas_allocator* Allocator);

atlas_alloc_id Atlas_Allocator_Alloc(atlas_allocator* Allocator, uvec2 Dim) {
    if(Dim.x == 0 || Dim.y == 0) {
        //Cannot allocate into the atlas if we don't request any space
        return {};
    }

    Dim.w = Align_U32(Allocator->Alignment.w, Dim.w);
    Dim.h = Align_U32(Allocator->Alignment.h, Dim.y);
    
    atlas_allocator_node* Node = Atlas__Find_Free_Rect(Allocator, Dim);
    if(!Node) {
        //Cannot find a free rect. Atlas is full!
        return {};
    }

    rect2u Rect(Node->Rect.Min, Node->Rect.Min+Dim);
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);

    rect2u SplitRect;
    rect2u LeftoverRect;
    b32    IsVertical;
    Atlas__Guillotine_Rect(&Node->Rect, Dim, Node->IsVertical, &SplitRect, &LeftoverRect, &IsVertical);

    atlas_allocator_node* AllocatedNode = nullptr;
    atlas_allocator_node* SplitNode = nullptr;
    atlas_allocator_node* LeftoverNode = nullptr;

    if(IsVertical == Node->IsVertical) {
        if(!Rect2u_Is_Empty(SplitRect)) {
            atlas_allocator_node* NextSibling = Node->Next;
            SplitNode = Atlas__Create_Node(Allocator);

            SplitNode->Parent     = Node->Parent;
            SplitNode->Next       = NextSibling;
            SplitNode->Prev       = Node;
            SplitNode->Rect       = SplitRect;
            SplitNode->IsVertical = Node->IsVertical;
            SplitNode->Type       = ATLAS_ALLOCATOR_NODE_TYPE_FREE;

            Node->Next = SplitNode;
            if(NextSibling) {
                NextSibling->Prev = SplitNode;
            }
        }

        if(!Rect2u_Is_Empty(LeftoverRect)) {
            Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;

            AllocatedNode = Atlas__Create_Node(Allocator);
            LeftoverNode  = Atlas__Create_Node(Allocator);

            AllocatedNode->Parent = Node;
            AllocatedNode->Next = LeftoverNode;
            AllocatedNode->Prev = nullptr;
            AllocatedNode->Rect = Rect;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical  = !Node->IsVertical;

            LeftoverNode->Parent = Node;
            LeftoverNode->Next = nullptr;
            LeftoverNode->Prev = AllocatedNode;
            LeftoverNode->Rect = LeftoverRect;
            LeftoverNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            LeftoverNode->IsVertical  = !Node->IsVertical;
        } else {
            AllocatedNode = Node;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->Rect = Rect;
        }
    } else {
        Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;

        if(!Rect2u_Is_Empty(SplitRect)) {
            SplitNode = Atlas__Create_Node(Allocator);
            SplitNode->Parent = Node;
            SplitNode->Next = nullptr;
            SplitNode->Prev = nullptr;
            SplitNode->Rect = SplitRect;
            SplitNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            SplitNode->IsVertical = !Node->IsVertical;
        }

        if(!Rect2u_Is_Empty(LeftoverRect)) {
            Assert(SplitNode);
            atlas_allocator_node* ContainerNode = Atlas__Create_Node(Allocator);
            ContainerNode->Parent = Node;
            ContainerNode->Next = SplitNode;
            ContainerNode->Prev = nullptr;
            ContainerNode->Rect = Rect2u_Empty();
            ContainerNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;
            ContainerNode->IsVertical = !Node->IsVertical;

            SplitNode->Prev = ContainerNode;

            AllocatedNode = Atlas__Create_Node(Allocator);
            LeftoverNode  = Atlas__Create_Node(Allocator);

            AllocatedNode->Parent = ContainerNode;
            AllocatedNode->Next = LeftoverNode;
            AllocatedNode->Prev = nullptr;
            AllocatedNode->Rect = Rect;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical = Node->IsVertical;

            LeftoverNode->Parent = ContainerNode;
            LeftoverNode->Next = nullptr;
            LeftoverNode->Prev = AllocatedNode;
            LeftoverNode->Rect = LeftoverRect;
            LeftoverNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            LeftoverNode->IsVertical = Node->IsVertical;
        } else {
            AllocatedNode = Atlas__Create_Node(Allocator);
            AllocatedNode->Parent = Node;
            AllocatedNode->Next   = SplitNode;
            AllocatedNode->Prev   = nullptr;
            AllocatedNode->Rect   = Rect;
            AllocatedNode->Type   = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical = !Node->IsVertical;

            SplitNode->Prev = AllocatedNode;
        }
    }

    Assert(AllocatedNode->Type == ATLAS_ALLOCATOR_NODE_TYPE_ALLOC);
    if(SplitNode) {
        Atlas__Add_Free_Rect(Allocator, SplitNode, Rect2u_Get_Dim(SplitRect));
    }

    if(LeftoverNode) {
        Atlas__Add_Free_Rect(Allocator, LeftoverNode, Rect2u_Get_Dim(LeftoverRect));
    }

#ifdef DEBUG_BUILD
    Atlas__Check_Tree(Allocator);
#endif

    return {
        .Generation = AllocatedNode->Generation,
        .Index      = AllocatedNode
    };
}

void Atlas_Allocator_Free(atlas_allocator* Allocator, atlas_alloc_id AllocID) {
    atlas_allocator_node* Node = (atlas_allocator_node*)Atlas_Allocator_Get(Allocator, AllocID);
    if(Node) {
        Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_ALLOC);
        Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;

        for(;;) {
            b32 IsVertical = Node->IsVertical;
            
            atlas_allocator_node* Next = Node->Next;
            atlas_allocator_node* Prev = Node->Prev;

            if(Next && Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Atlas__Merge_Siblings(Allocator, Node, Next, IsVertical);
            }

            if(Prev && Prev->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Atlas__Merge_Siblings(Allocator, Prev, Node, IsVertical);
                Node = Prev;
            }

            //If the node is a unique child. We try and collapse it
            atlas_allocator_node* Parent = Node->Parent;
            if(!Node->Prev && !Node->Next && Parent) {
                Assert(Parent->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER);

                Parent->Rect = Node->Rect;
                Parent->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
                Atlas__Delete_Node(Allocator, Node);
                Node = Parent;
            } else {
                Atlas__Add_Free_Rect(Allocator, Node, Rect2u_Get_Dim(Node->Rect));
                break;
            }
        }

#ifdef DEBUG_BUILD
        Atlas__Check_Tree(Allocator);
#endif
    }
}

atlas_index* Atlas_Allocator_Get(atlas_allocator* Allocator, atlas_alloc_id AllocID) {
    if(!AllocID.Generation || !AllocID.Index) return nullptr; 
    
    atlas_allocator_node* Node = (atlas_allocator_node*)AllocID.Index;
    if((Node->Type != ATLAS_ALLOCATOR_NODE_TYPE_ALLOC) || 
       (Node->Generation != AllocID.Generation)) {
        return nullptr;
    }

    return AllocID.Index;
}